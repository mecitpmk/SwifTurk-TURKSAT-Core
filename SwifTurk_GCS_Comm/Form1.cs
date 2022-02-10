using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;
using System.Diagnostics;

namespace SwifTurk_GCS_Comm
{
	public partial class Form1 : Form
	{

		public enum Command_Types : byte
		{
			MANUAL_RELEASE = 0,
			TEST_MOTOR	   = 1
		};

		public enum Package_Types : byte
		{
			TELEM_1HZ				= 0,
			TELEM_FLIGHT_HEALTH		= 1,
			GCS_TELEM_REQUEST		= 2,
			GCS_TELEM_MISSION		= 3,
			GCS_TELEM_COMMAND		= 4,
			GCS_TELEM_RESPONSE		= 5,
			CONTAINER_TELEMETRY		= 6,
			VIDEO_PACKAGE_REQUEST	= 7,
			UNKNOWN
		};
		
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		public unsafe struct Telemetry_1HZ_t
		{
			/* Main Telemetry Strcuture , Takes Sensore Data and
			 * Collect Container Telemetry Data.
			 */


			public uint START_OF_FRAME; // <
			public byte PACKAGE_TYPE; // this is 1Hz telemetry Structure @param => enum PACKAGE_TYPES
			public byte PACKAGE_SIZE;
			public ushort TEAM_ID;
			public ushort PACKET_NUMBER;  // will be increased every Hz



			public float Temperature;
			public float Pressure;
			public float Altitude;  // relative to Y-Axis
			public float DescentSpeed;  // relative to Y-Axis

			public float GPS_Latitude;
			public float GPS_Longtitude;
			public float GPS_Altitude;

			public float Container_Altitude;
			public float Container_Pressure;
			public float Container_GPS_Latitude;
			public float Container_GPS_Longtitude;
			public float Container_GPS_Altitude;
			public float AltitudeDifference;

			public float pitch;
			public float roll;
			public float yaw;

			public byte FLIGHT_STATUS;                          // for GCS Define => enum FLIGHT_STATUS_t : byte
			public byte VIDEO_TRANSMISSION_STATUS;  //

			public uint END_OF_FRAME; // >
		};

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		public unsafe struct GCS_Comamnd_t
		{
			public uint START_OF_FRAME; // <

			public byte PACKAGE_TYPE;

			public byte COMMAND;

			public uint END_OF_FRAME; // >
		};

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		public unsafe struct GCS_Video_t
		{
			public uint START_OF_FRAME; // <

			public byte PACKAGE_TYPE;

			public ushort videoID;

			public unsafe fixed byte videoByte[255]; // Bunu 255 e çek. ve yukarıya uint8_t bir videoSize koy. yani ornegin 255 byte lık bir array'de (Sonlara doğru) belki son paket
																	// 56 bytelık bir pakettir atıyorum. o yüzden böyle bir önlem al.(Payload okurken sadece 0 to 56 okuyacak.)

			public byte isEnd;
			public uint END_OF_FRAME; // >
		};


		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		public unsafe struct PayloadVideoHandler_t
		{
			public uint START_OF_FRAME; // <

			public byte PACKAGE_TYPE;

			public ushort videoID;

			public uint END_OF_FRAME; // >
		};

		public byte[] videoBuffer = new byte[3500];

		public static GCS_Video_t				gcsVideoPacket;
		public static GCS_Comamnd_t			    gcsCommandPacket;
		public static Telemetry_1HZ_t			Telemetry_1HZ_Packet;
		public static PayloadVideoHandler_t	    PayloadVideoHandlerPacket; 

		public static bool isSerialBusy	= false;
		public static bool readedAVI		= false;
		public static bool readignStarted	= false;
		public static bool isRdngCompleted = false;
		public static bool readingAllowed	= true;

		public static volatile bool isSerialSenderBusy	 = false;
		public static volatile bool isVideoSendingEnable = false;
		public static volatile bool waitforConformation	 = false;


		public static byte[] packetArr = new byte[150];
		public static int packetArrCt = 0;
		public static volatile int testVideoCt = 0;
		public static byte[] packetArr_TEST = new byte[150];


		public static Stopwatch timePassedWatch = new Stopwatch();
		public static Stopwatch timePassedWatchVideo = new Stopwatch();

		string[] SATELLEITE_STATE_STR    = { "Bekleme", "Yukselme", "Model Uydu Inis", "Ayrilma", "Gorev Yuku Inis", "Irtifa Sabit", "Kurtarma" };
		
		
		public static Thread videoSenderThread;

		public Form1()
        {
            InitializeComponent();
			initStructures();
			string[] ports = SerialPort.GetPortNames();

			Console.WriteLine("The following serial ports were found:");

			// Display each port name to the console.
			foreach (string port in ports)
			{
				Console.WriteLine(port);
			}
		}

		public unsafe void initStructures()
        {
			// Note :  gcsVideoPacket.videoByte fixed olduğu için, fonksiyonlarda değerini değiştirmek için
				// o tanımlı fonksiyon unsafe olmalı.
			// GcsVideoPacket
			gcsVideoPacket.START_OF_FRAME	= 0x5A5A5A5A;
			gcsVideoPacket.END_OF_FRAME		= 0x5A5A5A5A;
			gcsVideoPacket.isEnd			= 0;
			gcsVideoPacket.PACKAGE_TYPE		= (byte)Package_Types.GCS_TELEM_MISSION;
			gcsVideoPacket.videoID			= 1;
			gcsVideoPacket.videoByte[0]		= 0;
			gcsVideoPacket.videoByte[20]	= 22;


			gcsCommandPacket.START_OF_FRAME = 0x5A5A5A5A;
			gcsCommandPacket.END_OF_FRAME	= 0x5A5A5A5A;
			gcsCommandPacket.PACKAGE_TYPE	= (byte)Package_Types.GCS_TELEM_COMMAND;
			gcsCommandPacket.COMMAND		= (byte)Command_Types.MANUAL_RELEASE;
			//testPrint();

			
			//int size =  Marshal.SizeOf(gcsVideoPacket);
			//Console.WriteLine(size);
		}

		public unsafe void testPrint()
        {
			for (byte i = 0; i < 200; i++)
			{
				Console.WriteLine($"index = {i} , ValueofArr : {gcsVideoPacket.videoByte[i]}");
			}
		}

		public void clearBUFFER()
        {
			for (byte i = 0; i < 150; i++)
            {
				packetArr[i] = 0;
            }
        }
		public void solveTelemetryPackage(object sender, EventArgs e) //object sender , EventArgs e

		{
			Console.WriteLine($"Data package is solving......{packetArrCt}");
			IntPtr emptyPointer = Marshal.AllocHGlobal(packetArrCt);
			Marshal.Copy(packetArr, 0, emptyPointer, packetArrCt);
			if ( packetArr[4] == (byte)Package_Types.TELEM_1HZ )
            {
				//Console.WriteLine("Telemetry Package TRYING TO UNPACKAGE.");
				Telemetry_1HZ_Packet = (Telemetry_1HZ_t)Marshal.PtrToStructure(emptyPointer, Telemetry_1HZ_Packet.GetType());
				Console.WriteLine($"Telemetry Package has been unpackaged. ELapsed : {timePassedWatch.ElapsedMilliseconds}");
				timePassedWatch.Restart();
				Console.WriteLine($"Package No : {Telemetry_1HZ_Packet.PACKET_NUMBER} , State : {SATELLEITE_STATE_STR[Telemetry_1HZ_Packet.FLIGHT_STATUS]}");
				// Grafiklere bas.
			}
			else if ( packetArr[4] == (byte)Package_Types.VIDEO_PACKAGE_REQUEST )
            {
				videoTimer.Stop();
				Console.WriteLine("Payload Video Package TRYING TO UNPACKAGE.");
				PayloadVideoHandlerPacket = (PayloadVideoHandler_t)Marshal.PtrToStructure(emptyPointer, PayloadVideoHandlerPacket.GetType());
				Console.WriteLine($"PayloadVideoHandler  Package has been unpackaged. Req ID : {PayloadVideoHandlerPacket.videoID}");
				// Payload tarafından talep edilen ID bizim son gönderdiğimiz videoID+1 ise yeni videoByte gönder.
				if (PayloadVideoHandlerPacket.videoID == gcsVideoPacket.videoID+1)
                {
					Console.WriteLine("Gonderilen Video Alinmis Yenisi Gonderiliyor..");
					gcsVideoPacket.videoID += 1;
					testVideoCt += 1;
					//if (testVideoCt == 256 )
     //               {
					//	isVideoSendingEnable = false;
					//	gcsVideoPacket.isEnd = 1;
     //               }
                }
                waitforConformation = false;
            }
			
			Marshal.FreeHGlobal(emptyPointer);

			clearBUFFER();
			packetArrCt		= 0;
			isRdngCompleted = false;
			isSerialBusy	= false;
			readingAllowed  = true;
		}
        public unsafe void threadFunction ()
		{
			bool findAgain = false;
			for (; ; )
            {
				if (serialPortComm.BytesToRead > 0 && readingAllowed)
				{
					byte readedByte = (byte)serialPortComm.ReadByte();
                    packetArr[packetArrCt] = readedByte;
                    //Console.WriteLine($"Readed byte : {readedByte}");
                    if (packetArrCt >= 3)
                    {
						uint FrameChecker = (uint)(packetArr[packetArrCt-3] << 24 | packetArr[packetArrCt-2] << 16 | packetArr[packetArrCt-1] << 8 | packetArr[packetArrCt]);
						if (FrameChecker == 0x5A5A5A5A)
                        {
							Console.WriteLine("Lan 5a5a5a5a Buldu !");
							if (findAgain)
                            {
                                //Console.WriteLine($"Packet Array Counter : {packetArrCt}");
                                readingAllowed = false;
                                findAgain = false;
                                this.Invoke(new EventHandler(solveTelemetryPackage));

                                //packetArrCt = 0;// temporaraly
								continue;
                            }
							findAgain = true;
						}
						
					}
					packetArrCt += 1;

				}
			}
			
        }
        private void serialStartButton_Click(object sender, EventArgs e)
        {
            serialPortComm.Open();
            Thread pollingSerialFunc = new Thread(new ThreadStart(threadFunction));
			pollingSerialFunc.Start();
		}
		

		public void serialSenderFunction( Package_Types whichPackage )
        {
			isSerialSenderBusy = true;

			int structureSize = 0;
			byte[] byteArr;
			IntPtr ptr;

			switch ( whichPackage )
            {
				case Package_Types.GCS_TELEM_COMMAND:
					structureSize = Marshal.SizeOf(gcsCommandPacket);
					break;
				case Package_Types.GCS_TELEM_MISSION:
					structureSize = Marshal.SizeOf(gcsVideoPacket);
					break;
				default:
					break;
            }

			byteArr = new byte[structureSize];
			ptr = Marshal.AllocHGlobal(structureSize);
			
			if (whichPackage == Package_Types.GCS_TELEM_COMMAND)
            {
                Marshal.StructureToPtr(gcsCommandPacket, ptr, true);
            }
			else
            {
				Marshal.StructureToPtr(gcsVideoPacket, ptr, true);
			}
            Marshal.Copy(ptr, byteArr, 0, structureSize);
            Marshal.FreeHGlobal(ptr);

			serialPortComm.Write(byteArr, 0, byteArr.Length);

			isSerialSenderBusy = false;
		}
		private void releaseCommandButton_Click(object sender, EventArgs e)
        {
			if (isVideoSendingEnable)
            {
				videoSenderThread.Suspend();
			}
			gcsCommandPacket.COMMAND = (byte)Command_Types.MANUAL_RELEASE;
			serialSenderFunction( Package_Types.GCS_TELEM_COMMAND );
			if (isVideoSendingEnable)
            {
				videoSenderThread.Resume();
			}

        }


		public void readAVI()
        {
			// okunan byte'ları videoBuffer'a kopyle.
			return;
        }

		public unsafe int bufferSwap()
        {
			// Burda videoBuffer'dan al, gcsVideoPacket.videoByte'a aktar.
			
			for (byte i = 59; i < 250; i++)
            {
				gcsVideoPacket.videoByte[i] = i;
            }

			if (testVideoCt == 1176)
            {
				timePassedWatchVideo.Stop();
				Console.WriteLine($"Elapsed Video Sending Process 255 x 256: {timePassedWatchVideo.ElapsedMilliseconds}");
				gcsVideoPacket.isEnd = 1;
				isVideoSendingEnable = false;
				return 0;
				//videoSenderThread.Suspend();
            }
			return 1;
        }
		public unsafe void videoSenderFunction()
        {
			timePassedWatchVideo.Start();
			for (; ; )
            {
				if ( ( false == isSerialSenderBusy && true == isVideoSendingEnable ) && false == waitforConformation ) // Belki sonuncu  paketi almadı?
				{
					waitforConformation = true;
					bufferSwap();
					serialSenderFunction(Package_Types.GCS_TELEM_MISSION);
                    Console.WriteLine($"sender:{isSerialSenderBusy} , vidEn {isVideoSendingEnable}, conf:{waitforConformation} , isEnd : {gcsVideoPacket.isEnd} , CT : {testVideoCt}");
                }
			}
			
		}
        private void sendVideoButton_Click(object sender, EventArgs e)
        {
			if ( !readedAVI )
            {
				readAVI();

				isVideoSendingEnable = true;
				waitforConformation = false;
				videoSenderThread = new Thread(new ThreadStart(videoSenderFunction));
                videoSenderThread.Start();
				videoTimer.Enabled = true;
				videoTimer.Stop();

				readedAVI = true;
			}
			
		}

        private void videoTimer_Tick(object sender, EventArgs e)
        {
			Console.WriteLine("Ticking**************************");
			waitforConformation = false;
        }
    }
}
