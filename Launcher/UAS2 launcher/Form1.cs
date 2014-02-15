 ﻿/* 
  * This file is part of UAS2. 
  * 
  * UAS2 is free software; you can redistribute it and/or modify 
  * it under the terms of the GNU General Public License as published by 
  * the Free Software Foundation; either version 2 of the License, or 
  * (at your option) any later version. 
  * 
  * UAS2 is distributed in the hope that it will be useful, 
  * but WITHOUT ANY WARRANTY; without even the implied warranty of 
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
  * GNU General Public License for more details. 
  
  * You should have received a copy of the GNU General Public License 
  * along with UAS2; if not, write to the Free Software 
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
  *  
  * Author:  blakine 
  * Date:    10/02/2009 
  *  
 */


using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace UAS2_launcher
{
    public partial class frmMain : Form
    {
        public frmMain()
        {
            InitializeComponent();
            txtIP.Text = UAS2_launcher.Properties.Settings.Default.strIP;
            txtPort.Text = UAS2_launcher.Properties.Settings.Default.strPort;
            txtUser.Text = UAS2_launcher.Properties.Settings.Default.strUser;
            txtPass.Text = UAS2_launcher.Properties.Settings.Default.strPass;
        }

        private void btnExit_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void chkSave_CheckedChanged(object sender, EventArgs e)
        {
            if (chkSave.Checked)
            {
                UAS2_launcher.Properties.Settings.Default.strIP = txtIP.Text;
                UAS2_launcher.Properties.Settings.Default.strPort = txtPort.Text;
                UAS2_launcher.Properties.Settings.Default.strUser = txtUser.Text;
                UAS2_launcher.Properties.Settings.Default.strPass = txtPass.Text;
                UAS2_launcher.Properties.Settings.Default.Save();
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.FileName = "client.exe";
            startInfo.UseShellExecute = false;
            startInfo.RedirectStandardOutput = true;
            string start = " -h " + txtIP.Text.ToString() + " -p " + txtPort.Text.ToString() + " -a " + txtUser.Text.ToString() + ":" + txtPass.Text.ToString();
            startInfo.Arguments = start;
            Process.Start(startInfo);
        }


    }
}
