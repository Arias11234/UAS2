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
using MySql.Data.MySqlClient;
using System.Data;
using System.Text;
using System.Windows.Forms;
using DBprop = Vendor_Editor.Properties.Settings;

namespace Vendor_Editor
{
    public class Database
    {
        //public List<string> vendor_names = new List<string>();
        public List<string> item_names = new List<string>();
        public List<int> vendor_inv_items = new List<int>();
        public List<string> item_types = new List<string>();
        public IDictionary<int, string> vendor_names = new Dictionary<int, string>();
        public IDictionary<int, int> itemtypes = new Dictionary<int, int>();

        //Intialize
        public string InitializeDB()
        {
            string ConStr;
            return ConStr = "SERVER=" + DBprop.Default.ServerIP + ";PORT=" + DBprop.Default.ServerPort + ";DATABASE=" + DBprop.Default.DBName + ";UID=" + DBprop.Default.DBUser + ";PWD=" + DBprop.Default.DBPassword + ";";   
        }

        /// <summary>
        /// MySQLCommandEntry
        /// </summary>
        /// <param name="command">The SQL Command string to execute</param>
        /// <param name="conn">The SQL Connection to use</param>
        /// <param name="value">The value to retrieve from the database</param>
        /// <returns>value</returns>
        public int MySQLCommandEntry(string command, MySqlConnection conn, int value)
        {
            MySqlCommand my = new MySqlCommand(command, conn);
            value = (int)my.ExecuteScalar();
            return value;
        }
        /// <summary>
        /// FindValueInDB - Looks for a specific value in a specific column in the database.
        /// </summary>
        /// <param name="tablename">The SQL table name to run the query on</param>
        /// <param name="qualifier">The column in the table to look in</param>
        /// <param name="column">The column name variable</param>
        /// <param name="value">The value to look for</param>
        /// <param name="conn">The connection object</param>
        /// <param name="reader">The reader object</param>
        /// <returns>True if value was found</returns>
        public bool FindValueInDB(string tablename, string qualifier, string column, string value, MySqlConnection conn)
        {
            MySqlCommand find = new MySqlCommand("Select * from " + tablename + " where " + qualifier + "= ?" + value, conn);
            find.Parameters.AddWithValue("?" + value, column);
            MySqlDataReader reader = find.ExecuteReader();

            if (!reader.Read())
            {
                reader.Close();
                return false;
            }
            reader.Close();
            return true;
        }
        /// <summary>
        /// FindValueInDB - Looks for a specific value in a specific column in the database.
        /// </summary>
        /// <param name="tablename">The SQL table name to run the query on</param>
        /// <param name="qualifier">The column in the table to look in</param>
        /// <param name="column">The column name variable</param>
        /// <param name="value">The value to look for</param>
        /// <param name="conn">The connection object</param>
        /// <param name="reader">The reader object</param>
        /// <returns>True if value was found</returns>
        public bool FindValueInDB(string tablename, string qualifier, int column, int value, MySqlConnection conn)
        {
            MySqlCommand find = new MySqlCommand("Select * from " + tablename + " where " + qualifier + "= ?" + value, conn);
            find.Parameters.AddWithValue("?" + value, column);
            MySqlDataReader reader = find.ExecuteReader();

            if (!reader.Read())
            {
                reader.Close();
                return false;
            }
            reader.Close();
            return true;
        }

        public int FindNPCID(MySqlConnection conn, string npc_name)
        {
            int npc_id;
            MySqlCommand findnpcid = new MySqlCommand("Select npc_id from npcs where Name = ?npc_name", conn);
            findnpcid.Parameters.AddWithValue("?npc_name", npc_name);
            MySqlDataAdapter da = new MySqlDataAdapter();
            MySqlDataReader reader = findnpcid.ExecuteReader();

            if (!reader.Read())
            {
                reader.Close();
                return 0;             
            }
            else
            {
                npc_id = reader.GetInt16("npc_id");
                reader.Close();
                
            }
            return npc_id;
        }

        public int FindItemID(MySqlConnection conn, string itemname)
        {
            int itemid = 0;
            
            MySqlCommand finditemid = new MySqlCommand("Select dwlinker from items_templates where name = ?itemname", conn);
            finditemid.Parameters.AddWithValue("?itemname", itemname);
            MySqlDataReader reader = finditemid.ExecuteReader();

            if (!reader.Read())
            {
                reader.Close();
                return 0;
            }
            else
            {
                itemid = reader.GetInt16("dwLinker");
                reader.Close();
                return itemid;
            }

        }

        public string FindItemName(MySqlConnection conn, int itemID)
        {
            string itemname;
            MySqlCommand finditemid = new MySqlCommand("Select Name from items_templates where dwLinker = ?" + itemID, conn);
            finditemid.Parameters.AddWithValue("?" + itemID, itemID);
            MySqlDataReader reader = finditemid.ExecuteReader();

            if (!reader.Read())
            {
                reader.Close();
                return null;
            }
            else
            {
                itemname = reader.GetString("Name");
                reader.Close();
                return itemname;
            }

        }
        public void LoadVendors(MySqlConnection conn)
        {
            int isvendor = 1;

            MySqlCommand getvendors = new MySqlCommand("SELECT * FROM npcs where isVendor = ?" + isvendor, conn);
            getvendors.Parameters.AddWithValue("?" + isvendor, isvendor);
            MySqlDataAdapter da = new MySqlDataAdapter();
            da.SelectCommand = getvendors;
            DataSet ds = new DataSet();
            DataTable dt = new DataTable();
            da.Fill(ds);
            dt = ds.Tables[0];

            foreach (DataRow dr in dt.Rows)
            {
                 vendor_names.Add(Convert.ToInt16(dr["npc_id"]),dr["Name"].ToString());
            }
        }

        public void LoadVendorInventory(MySqlConnection conn, int vendor_id)
        {
            MySqlCommand getvendors = new MySqlCommand("SELECT * FROM npcs_vendor_items where vendor_id = ?" + vendor_id, conn);
            getvendors.Parameters.AddWithValue("?" + vendor_id, vendor_id);
            MySqlDataAdapter da = new MySqlDataAdapter();
            da.SelectCommand = getvendors;
            DataSet ds = new DataSet();
            DataTable dt = new DataTable();
            da.Fill(ds);
            dt = ds.Tables[0];

            foreach (DataRow dr in dt.Rows)
            {
                vendor_inv_items.Add(Convert.ToInt16(dr["dwlinker"]));
            }
        }


        public void LoadDBItems(MySqlConnection conn)
        {


            MySqlCommand getitems = new MySqlCommand("SELECT * FROM items_templates", conn);
            MySqlDataAdapter da = new MySqlDataAdapter();
            da.SelectCommand = getitems;
            DataSet ds = new DataSet();
            DataTable dt = new DataTable();
            da.Fill(ds);
            dt = ds.Tables[0];

            foreach (DataRow dr in dt.Rows)
            {
                item_names.Add(dr["Name"].ToString());
            }
        }

        public void LoadItemTypes(MySqlConnection conn, int itemtype)
        {
            MySqlCommand getitemtypes = new MySqlCommand("SELECT * FROM items_templates where itemtype = ?" + itemtype, conn);
            getitemtypes.Parameters.AddWithValue("?" + itemtype, itemtype);
            MySqlDataAdapter da = new MySqlDataAdapter();
            da.SelectCommand = getitemtypes;
            DataSet ds = new DataSet();
            DataTable dt = new DataTable();
            da.Fill(ds);
            dt = ds.Tables[0];

            foreach (DataRow dr in dt.Rows)
            {
                item_types.Add(dr["Name"].ToString());
            }
        }
    }
}
