
namespace SwifTurk_GCS_Comm
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.serialStartButton = new System.Windows.Forms.Button();
            this.releaseCommandButton = new System.Windows.Forms.Button();
            this.sendVideoButton = new System.Windows.Forms.Button();
            this.serialPortComm = new System.IO.Ports.SerialPort(this.components);
            this.videoTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // serialStartButton
            // 
            this.serialStartButton.Location = new System.Drawing.Point(291, 82);
            this.serialStartButton.Name = "serialStartButton";
            this.serialStartButton.Size = new System.Drawing.Size(170, 23);
            this.serialStartButton.TabIndex = 0;
            this.serialStartButton.Text = "Communucation Start";
            this.serialStartButton.UseVisualStyleBackColor = true;
            this.serialStartButton.Click += new System.EventHandler(this.serialStartButton_Click);
            // 
            // releaseCommandButton
            // 
            this.releaseCommandButton.Location = new System.Drawing.Point(66, 134);
            this.releaseCommandButton.Name = "releaseCommandButton";
            this.releaseCommandButton.Size = new System.Drawing.Size(228, 23);
            this.releaseCommandButton.TabIndex = 1;
            this.releaseCommandButton.Text = "Release Payload Command";
            this.releaseCommandButton.UseVisualStyleBackColor = true;
            this.releaseCommandButton.Click += new System.EventHandler(this.releaseCommandButton_Click);
            // 
            // sendVideoButton
            // 
            this.sendVideoButton.Location = new System.Drawing.Point(458, 134);
            this.sendVideoButton.Name = "sendVideoButton";
            this.sendVideoButton.Size = new System.Drawing.Size(228, 23);
            this.sendVideoButton.TabIndex = 2;
            this.sendVideoButton.Text = "Start Send Video";
            this.sendVideoButton.UseVisualStyleBackColor = true;
            this.sendVideoButton.Click += new System.EventHandler(this.sendVideoButton_Click);
            // 
            // serialPortComm
            // 
            this.serialPortComm.BaudRate = 115200;
            this.serialPortComm.PortName = "COM9";
            // 
            // videoTimer
            // 
            this.videoTimer.Tick += new System.EventHandler(this.videoTimer_Tick);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.sendVideoButton);
            this.Controls.Add(this.releaseCommandButton);
            this.Controls.Add(this.serialStartButton);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button serialStartButton;
        private System.Windows.Forms.Button releaseCommandButton;
        private System.Windows.Forms.Button sendVideoButton;
        private System.IO.Ports.SerialPort serialPortComm;
        public System.Windows.Forms.Timer videoTimer;
    }
}

