/*
 * This file is part of Vendor Editor.
 *
 * Vendor Editor is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vendor Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Vendor Editor; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Author:  blakine
 * Date:    07/22/2009
 * 
*/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using MySql.Data.MySqlClient;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace Vendor_Editor
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        int itemcount = 0;
        int items_added_count = 0;

        DBsettings db = new DBsettings();
        AboutBox1 ab = new AboutBox1();
        Database dbase = new Database();

        public static ListBox lstDBItems = new ListBox();

        private void Form1_Load(object sender, EventArgs e)
        {
            lstDBItems.ForeColor = System.Drawing.Color.Black;
            lstDBItems.FormattingEnabled = true;
            lstDBItems.Location = new System.Drawing.Point(281, 97);
            lstDBItems.Name = "lstDBItems";
            lstDBItems.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            lstDBItems.Size = new System.Drawing.Size(225, 199);
            lstDBItems.TabIndex = 11;
            lstDBItems.Sorted = true;
            lstDBItems.DoubleClick += new System.EventHandler(this.lstDBItems_DoubleClick);
            this.Controls.Add(lstDBItems);

            MySqlConnection dbConn = new MySqlConnection(dbase.InitializeDB());
            try
            {
                dbConn.Open();
                dbase.LoadVendors(dbConn);
                dbase.LoadDBItems(dbConn);

                foreach (KeyValuePair<int, string> kvp in dbase.vendor_names)
                {
                    cmbVendorName.Items.Add(kvp.Value.ToString());
                }
                 
                foreach (string s in dbase.item_names)
                {
                    lstDBItems.Items.Add(dbase.item_names[itemcount]);
                    itemcount++;
                }

                cmbItemType.Items.Insert(0, "General Items");
                cmbItemType.Items.Insert(1, "Weapons");
                cmbItemType.Items.Insert(2, "Food, Potions");
                cmbItemType.Items.Insert(3, "Armor");
                cmbItemType.Items.Insert(4, "Books");
                cmbItemType.Items.Insert(5, "Scrolls");
                cmbItemType.Items.Insert(6, "Healing Kits");
                cmbItemType.Items.Insert(7, "Lockpicks, Keys");
                cmbItemType.Items.Insert(8, "Wands, Orbs, Staves");
                cmbItemType.Items.Insert(9, "Mana Stones, Mana Charges");
                cmbItemType.Items.Insert(10, "Ammunition");
                cmbItemType.Items.Insert(11, "Shields");
                cmbItemType.Items.Insert(12, "Spell Components");
                cmbItemType.Items.Insert(13, "Gems");
                cmbItemType.Items.Insert(14, "Trade Notes");
                cmbItemType.Items.Insert(15, "Clothes");
                cmbItemType.Items.Insert(16, "Jewelry");
                cmbItemType.Items.Insert(17, "Packs");
                cmbItemType.Items.Insert(18, "Foci");
                cmbItemType.Items.Insert(19, "Missile Weapons");
                cmbItemType.Items.Insert(20, "Fletching Items");
                cmbItemType.Items.Insert(21, "Alchemy Items");
                cmbItemType.Items.Insert(22, "Cooking Items");


                
            }
            catch (MySqlException sqlex)
            {
                string message = sqlex.Message;
                MessageBox.Show(message + "\n Please check your datbase settings", "MySQL Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            dbConn.Close();
        }

        private void SelectVendor(object sender, EventArgs e)
        {
            lstVendorItems.Items.Clear();

            for (int i = 0; i < lstVendorItems.Items.Count; i++)
            {
                lstVendorItems.Items.RemoveAt(i);
            }

            dbase.vendor_inv_items.Clear();
            MySqlConnection dbConn = new MySqlConnection(dbase.InitializeDB());
            dbConn.Open();
            int vendor_id = dbase.FindNPCID(dbConn, cmbVendorName.Text);
            dbase.LoadVendorInventory(dbConn, vendor_id);

            foreach (int i in dbase.vendor_inv_items)
            {
                lstVendorItems.Items.Add(dbase.FindItemName(dbConn, i));
            }
        }

        private void databaseSettingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            db.Show();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult result = MessageBox.Show("Are you sure you want to exit?", "Exit Application?", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            if(result == DialogResult.Yes)
                Application.Exit();
        }

        private void aboutVendorEditorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ab.Show();
        }

        private void btnAddItem_Click(object sender, EventArgs e)
        {
            MySqlConnection dbConn = new MySqlConnection(dbase.InitializeDB());
            dbConn.Open();
            int vendor_id = dbase.FindNPCID(dbConn, cmbVendorName.Text);
            DialogResult AddItems = MessageBox.Show("Add " + lstDBItems.SelectedItems.Count + " items to the vendors inventory?", "MySQL Insert", MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            if (AddItems == DialogResult.Yes)
            {
                try
                {
                    foreach (string s in lstDBItems.SelectedItems)
                    {
                        items_added_count++;
                        int dwlinker = dbase.FindItemID(dbConn, s);
                        string ItemName = dbase.FindItemName(dbConn, dwlinker);
                        MySqlCommand update_vendor_items = new MySqlCommand("INSERT INTO npcs_vendor_items(vendor_id,dwlinker,item_name) VALUES(" + vendor_id + "," + dwlinker + ",'" + ItemName + "'" + ")", dbConn);
                        update_vendor_items.ExecuteNonQuery();
                        
                    }
                    MessageBox.Show(lstDBItems.SelectedItems.Count + " items were added to the vendor items table.", "DB UPdate!", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    
                }
                catch (MySqlException sqlex)
                {
                    string message = sqlex.Message;
                    MessageBox.Show(message, "MySQL Error", MessageBoxButtons.OK, MessageBoxIcon.Error);

                }
                for (int i = 0; i < lstDBItems.SelectedItems.Count; i++)
                {
                    lstVendorItems.Items.Add(lstDBItems.SelectedItems[i]);
                }
                lstDBItems.SelectedItems.Clear();
            }
            dbConn.Close();

        }

        private void btnRemoveItem_Click(object sender, EventArgs e)
        {
            MySqlConnection dbConn = new MySqlConnection(dbase.InitializeDB());
            dbConn.Open();
            int vendor_id = dbase.FindNPCID(dbConn, cmbVendorName.Text);
            DialogResult RemoveItems = MessageBox.Show("Delete " + lstVendorItems.SelectedItems.Count + " items to the vendors inventory?", "MySQL Delete", MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            if (RemoveItems == DialogResult.Yes)
            {
                try
                {
                    foreach (string s in lstVendorItems.SelectedItems)
                    {
                        items_added_count++;
                        int dwlinker = dbase.FindItemID(dbConn, s);
                        string ItemName = lstVendorItems.Text;
                        MySqlCommand remove_vendor_items = new MySqlCommand("DELETE from npcs_vendor_items where dwlinker = ?dwlinker", dbConn);
                        remove_vendor_items.Parameters.AddWithValue("?dwlinker", dwlinker);
                        remove_vendor_items.ExecuteNonQuery();
                        
                    }
                    MessageBox.Show(lstVendorItems.SelectedItems.Count + " items were removed from the vendor items table.", "DB UPdate!", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    
                }
                catch (MySqlException sqlex)
                {
                    string message = sqlex.Message;
                    MessageBox.Show(message, "MySQL Error", MessageBoxButtons.OK, MessageBoxIcon.Error);

                }

                while (lstVendorItems.SelectedItems.Count > 0)
                {
                    lstVendorItems.Items.Remove(lstVendorItems.SelectedItem);
                }

            }
            dbConn.Close();
        }

        public int GetComboItemType(int index)
        {
            switch (index)
            {
                case 0:
                    return 0;
                case 1:
                    return 1;
                case 2:
                    return 2;
                case 3:
                    return 3;
                case 4:
                    return 4;
                case 5:
                    return 5;
                case 6:
                    return 6;
                case 7:
                    return 7;
                case 8:
                    return 8;
                case 9:
                    return 10;
                case 10:
                    return 11;
                case 11:
                    return 12;              
                case 12:
                    return 13;                 
                case 13:
                    return 14;                 
                case 14:
                    return 15;                 
                case 15:
                    return 18;                   
                case 16:
                    return 19;                
                case 17:
                    return 20;             
                case 18:
                    return 22;         
                case 19:
                    return 23;               
                case 20:
                    return 16;
                default:
                    return 0;
            }
        }

        private void lstDBItems_DoubleClick(object sender, EventArgs e)
        {
            MySqlConnection dbConn = new MySqlConnection(dbase.InitializeDB());
            dbConn.Open();
            int vendor_id = dbase.FindNPCID(dbConn, cmbVendorName.Text);

            try
            {
                int dwlinker = dbase.FindItemID(dbConn, lstDBItems.SelectedItem.ToString());
                string ItemName = dbase.FindItemName(dbConn, dwlinker);
                MySqlCommand update_vendor_items = new MySqlCommand("INSERT INTO npcs_vendor_items(vendor_id,dwlinker,item_name) VALUES(" + vendor_id + "," + dwlinker + ",'" + ItemName + "'" + ")", dbConn);
                update_vendor_items.ExecuteNonQuery();
            }
            catch (MySqlException sqlex)
            {
                string message = sqlex.Message;
                MessageBox.Show(message, "MySQL Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                dbConn.Close();
            }

            for (int i = 0; i < lstDBItems.SelectedItems.Count; i++)
            {
                lstVendorItems.Items.Add(lstDBItems.SelectedItems[i]);
            }
            lstDBItems.SelectedItems.Clear();
            dbConn.Close();
        }

        public void ClickHandler(Object sender, System.EventArgs e)
        {
            dbase.item_types.Clear();
            int item_type_count = 0;
            int itemtype = GetComboItemType(Convert.ToInt16(((ComboBox)sender).SelectedIndex));
            MySqlConnection dbConn = new MySqlConnection(dbase.InitializeDB());

                Form1.lstDBItems.Items.Clear();
                dbConn.Open();
                dbase.LoadItemTypes(dbConn, itemtype);
                foreach (string s in dbase.item_types)
                {
                    Form1.lstDBItems.Items.Add(dbase.item_types[item_type_count]);
                    item_type_count++;
                }
                dbConn.Close();
        }

    }
}

