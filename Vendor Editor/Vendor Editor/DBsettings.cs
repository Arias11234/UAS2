using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Vendor_Editor
{
    public partial class DBsettings : Form
    {
        public DBsettings()
        {
            InitializeComponent();
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Hide();
        }

        private void btnSave_Click(object sender, EventArgs e)
        {

            Vendor_Editor.Properties.Settings.Default.ServerIP = txtServerIP.Text;
            Vendor_Editor.Properties.Settings.Default.ServerPort = txtServerPort.Text;
            Vendor_Editor.Properties.Settings.Default.DBName = txtDBName.Text;
            Vendor_Editor.Properties.Settings.Default.DBUser = txtUsername.Text;
            Vendor_Editor.Properties.Settings.Default.DBPassword = txtPassword.Text;
            Vendor_Editor.Properties.Settings.Default.Save();
            this.Hide();
        }


    }
}
