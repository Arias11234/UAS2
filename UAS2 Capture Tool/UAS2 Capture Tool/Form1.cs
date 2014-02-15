using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Globalization;
using System.Diagnostics;
using MySql.Data.MySqlClient;

namespace UAS2_Capture_Tool
{
    public partial class frmMain : Form
    {
        public frmMain()
        {
            InitializeComponent();
            btnBeginScan.Enabled = false;
            chkAll.Checked = true;
            chkNoDb.Checked = false;
            openFolder = false;
        }

        public void initialize()
        {
            messageCount = 0;
            processed = 0;
            filesProcessed = 0;
            labelFileNum.Text = "";
            labelFileNum.Refresh();
            pbProgress.Equals(0);

            iChests = 0;
            iDoors = 0;
            iMerchantSigns = 0;
            iTownSigns = 0;
            iPortals = 0;
            iMonsterSpawns = 0;
            iMonsterPTM = 0;
            iNPCs = 0;
            iTroves = 0;
            iCovenants = 0;
            iApartments = 0;
            iCottages = 0;
            iVillas = 0;
            iMansions = 0;
            iHooks = 0;
            iHouseChests = 0;
            iLifestones = 0;
            iWells = 0;
            iFountains = 0;
            iItems = 0;
            iArmor = 0;
            iWeapons = 0;
            iAmmo = 0;
            iWands = 0;
            iClothes = 0;
            iJewelry = 0;
            iBooks = 0;
            iKeys = 0;
            iComps = 0;
            iManastones = 0;
            iFood = 0;
            iGems = 0;
            iNotes = 0;
            iPack = 0;
            iFletch = 0;
            iAlchemy = 0;
            iCook = 0;
            iSalvage = 0;
            iDecor = 0;
            iMisc = 0;
        }

        /* Globals */
        #region Global Variables

        string path;
        DirectoryInfo dir;
        StreamWriter sw;
        MySqlConnection dbConn;
        Boolean openFolder;

        int messageCount;
        int processed;
        int filesProcessed;

        //Object count variables
        #region Count Variables
        int iChests;
        int iDoors;
        int iMerchantSigns;
        int iTownSigns;
        int iPortals;
        int iMonsterSpawns;
        int iMonsterPTM;
        int iNPCs;
        int iTroves;
        int iCovenants;
        int iApartments;
        int iCottages;
        int iVillas;
        int iMansions;
        int iHooks;
        int iHouseChests;
        int iLifestones;
        int iWells;
        int iFountains;
        int iItems;
        int iArmor;
        int iWeapons;
        int iAmmo;
        int iWands;
        int iClothes;
        int iJewelry;
        int iBooks;
        int iKeys;
        int iComps;
        int iManastones;
        int iFood;
        int iGems;
        int iNotes;
        int iPack;
        int iFletch;
        int iAlchemy;
        int iCook;
        int iSalvage;
        int iDecor;
        int iMisc;
        #endregion

        private string ConStr;
        ushort wIcon;
        string wo2lb;
        string wo2x;
        string wo2y;
        string wo2z;
        string wo2xQuat;
        string wo2yQuat;
        string wo2wQuat;
        string wo2zQuat;
        #endregion

        //String arrays for p/t/m info
        #region NPC Strings
        /*      NPCS        */
        //Palette
        string[] sNewPalette = new string[100];
        string[] sPaletteOffset = new string[100];
        string[] sPaletteLength = new string[100];
        string fix_pal;
        string initial_line = "INSERT INTO npcs_vector_palettes VALUES";

        //Texture
        string[] sTModelIndex = new string[100];
        string[] sOldTexture1 = new string[100];
        string[] sOldTexture2 = new string[100];
        string[] sNewTexture1 = new string[100];
        string[] sNewTexture2 = new string[100];
        string insert_tex = "INSERT INTO npcs_vector_textures VALUES";

        //Models
        string[] sMModelIndex = new string[100];
        string[] sNewModel1 = new string[100];
        string[] sNewModel2 = new string[100];
        string insert_mod = "INSERT INTO npcs_vector_models VALUES";

        //String for updating NPC table
        string npc_name;

        //Strings for updating Model Data table
        string wModel;
        string wPaletteCode;

        //This is the PK from palettesvector table.
        uint c_npc_palette_id = 0;
        //This is the PK from texturesvector table.
        uint c_npc_texture_id = 0;
        //This is the PK from modelsvector table.
        uint c_npc_model_id = 0;

        //We also need the Modelvector, so we can increment it.
        uint c_npc_palette_mv = 0;
        uint c_npc_texture_mv = 0;
        uint c_npc_model_mv = 0;

        #endregion
        #region Monster Strings
        /*      Monsters        */
        //Palette
        string[] m_NewPalette = new string[50];
        string[] m_PaletteOffset = new string[50];
        string[] m_PaletteLength = new string[50];
        string m_fix_pal;
        string monster_pal_insert = "INSERT INTO monsters_vetors_palettes VALUES";

        //Texture
        string[] m_TModelIndex = new string[50];
        string[] m_OldTexture1 = new string[50];
        string[] m_OldTexture2 = new string[50];
        string[] m_NewTexture1 = new string[50];
        string[] m_NewTexture2 = new string[50];
        string monster_tex_insert = "INSERT INTO monsters_vetors_textures VALUES";

        //Models
        string[] m_MModelIndex = new string[50];
        string[] m_NewModel1 = new string[50];
        string[] m_NewModel2 = new string[50];
        string monster_mod_insert = "INSERT INTO monsters_vetors_models VALUES";

        //String for updating Monster table
        string monster_name;
        List<string> monster_name_array = new List<string>();
        Boolean sameName;

        //Strings for updating Monster Model Data table
        string m_Model;
        string m_Species;
        string m_Anim;
        string m_Sound;
        int m_res;

        //This is the PK from mpalettes table.
        uint c_mon_palette_id = 0;
        //This is the PK from mtextures table.
        uint c_mon_texture_id = 0;
        //This is the PK from mmodels table.
        uint c_mon_model_id = 0;

        //We also need the Modelvector, so we can increment it.
        uint c_mon_palette_mv = 0;
        uint c_mon_texture_mv = 0;
        uint c_mon_model_mv = 0;

        #endregion
        #region Item Strings
        /*      ITEMS       */

        //Palettes
        string[] Item_sNewPalette = new string[50];
        string[] Item_sPaletteOffset = new string[50];
        string[] Item_sPaletteLength = new string[50];
        string item_fix_pal;
        string Item_pal_insert = "INSERT INTO items_vetors_palettes VALUES";

        //Textures
        string[] Item_sTModelIndex = new string[50];
        string[] Item_sOldTexture1 = new string[50];
        string[] Item_sOldTexture2 = new string[50];
        string[] Item_sNewTexture1 = new string[50];
        string[] Item_sNewTexture2 = new string[50];
        string Item_tex_insert = "INSERT INTO items_vetors_textures VALUES";

        //Models
        string[] Item_sMModelIndex = new string[50];
        string[] Item_sNewModel1 = new string[50];
        string[] Item_sNewModel2 = new string[50];
        string Item_mod_insert = "INSERT INTO items_vetors_models VALUES";
        #endregion

        string item_name;
        short dwModelNumber;
        float scale;
        string s_w_Model;
        ushort wPortalMode = 0;
        ushort unknown1 = 0;
        int unk10 = 0;
        int unknown100 = 0;
        int unk8000000 = 0;
        int byteCount = 0;
        string unknownBytes;
        int ovalue = 0;
        int oburden = 0;
        int ammotype = 0;
        int useableon = 0;
        int iconhigh = 0;
        int wieldtype = 0;
        int uses = 0;
        int uselimit = 0;
        int stack = 0;
        int stacklimit = 0;
        int equippos = 0;
        int equipact = 0;
        int coverage = 0;
        int hooks = 0;

        //PKs and MVs
        uint c_item_palette_pk;
        uint c_item_texture_pk;
        uint c_item_model_pk;
        uint c_item_palette_mv;
        uint c_item_texture_mv;
        uint c_item_model_mv;

        //Functions
        #region MySQL Functions
        /// <summary>
        /// MySQLCommandEntry
        /// </summary>
        /// <param name="command">The SQL Command string to execute</param>
        /// <param name="conn">The SQL Connection to use</param>
        /// <param name="value">The value to retrieve from the database</param>
        /// <returns>value</returns>
        private uint MySQLCommandEntry(string command, MySqlConnection conn, uint value)
        {
            MySqlCommand my = new MySqlCommand(command, conn);
            value = (uint)my.ExecuteScalar();
            return value;
        }
        #endregion
        #region Model Functions
        /// <summary>
        /// Function to insert PTM for items.
        /// </summary>
        private void Item_PTM(uint pc, string pal_table, uint tc, string tex_table, uint mc, string mod_table, StreamWriter sw)
        {
            string pal_insert = "INSERT INTO " + pal_table + " VALUES";
            string tex_insert = "INSERT INTO " + tex_table + " VALUES";
            string mod_insert = "INSERT INTO " + mod_table + " VALUES";

            if (pc > 0)
            {
                sw.WriteLine(pal_insert);
                for (int j = 0; j < pc; j++)
                {
                    item_fix_pal = Item_sNewPalette[j].Replace("-", "");
                    sw.Write(item_fix_pal);
                    sw.Write(Item_sPaletteOffset[j]);
                    sw.Write(Item_sPaletteLength[j]);
                    sw.WriteLine();
                }
            }

            if (tc > 0)
            {
                sw.WriteLine(tex_insert);
                for (int j = 0; j < tc; j++)
                {
                    sw.Write(Item_sTModelIndex[j]);
                    sw.Write(Item_sNewTexture2[j]);
                    sw.Write(Item_sNewTexture1[j]);
                    sw.Write(Item_sOldTexture2[j]);
                    sw.Write(Item_sOldTexture1[j]);
                    sw.WriteLine();
                }
            }

            if (mc > 0)
            {
                sw.WriteLine(mod_insert);
                for (int j = 0; j < mc; j++)
                {
                    sw.Write(Item_sMModelIndex[j]);
                    sw.Write(Item_sNewModel2[j]);
                    sw.Write(Item_sNewModel1[j]);
                    sw.WriteLine();
                }
            }
            if (pc + tc + mc == 0)
            {
                c_item_palette_mv++;
            }
        }
        /// <summary>
        /// Function to insert PTM for monsters.
        /// </summary>
        private void Monster_PTM(uint pc, uint tc, uint mc, StreamWriter sw)
        {
            if (pc > 0)
            {
                sw.WriteLine(monster_pal_insert);
                for (int j = 0; j < pc; j++)
                {
                    m_fix_pal = Item_sNewPalette[j].Replace("-", "");
                    sw.Write(m_fix_pal);
                    sw.Write(m_PaletteOffset[j]);
                    sw.Write(m_PaletteLength[j]);
                    sw.WriteLine();
                }
            }

            if (tc > 0)
            {
                sw.WriteLine(monster_tex_insert);
                for (int j = 0; j < tc; j++)
                {
                    sw.Write(m_TModelIndex[j]);
                    sw.Write(m_NewTexture2[j]);
                    sw.Write(m_NewTexture1[j]);
                    sw.Write(m_OldTexture2[j]);
                    sw.Write(m_OldTexture1[j]);
                    sw.WriteLine();
                }
            }

            if (mc > 0)
            {
                sw.WriteLine(monster_mod_insert);
                for (int j = 0; j < mc; j++)
                {
                    sw.Write(m_MModelIndex[j]);
                    sw.Write(m_NewModel2[j]);
                    sw.Write(m_NewModel1[j]);
                    sw.WriteLine();
                }
            }
        }
        #endregion
        #region Checkmark Functions
        /* Checkmark Functions */
        private void chkNoDb_CheckedChanged(object sender, EventArgs e)
        {
            txtDBName.Enabled = false;
            txtUser.Enabled = false;
            txtPass.Enabled = false;
        }
        private void chkAll_CheckedChanged(object sender, EventArgs e)
        {
            if (chkAll.Checked)
            {
                chkAllWorld.Checked = true;
                chkAllHousing.Checked = true;
                chkAllSpawns.Checked = true;
                chkAllTown.Checked = true;
                chkAllItems.Checked = true;

                chkPortals.Checked = true;
                chkLifestones.Checked = true;
                chkDoors.Checked = true;
                chkWells.Checked = true;
                chkFountains.Checked = true;
                chkChests.Checked = true;
                chkTroves.Checked = true;

                chkCovenantCrystals.Checked = true;
                chkHouseChests.Checked = true;
                chkHooks.Checked = true;
                chkMansions.Checked = true;
                chkVillas.Checked = true;
                chkCottages.Checked = true;
                chkApartments.Checked = true;

                chkNPCs.Checked = true;
                chkMonSpawns.Checked = true;
                chkMonPTM.Checked = true;

                chkTownSigns.Checked = true;
                chkMerchantSigns.Checked = true;

                chkItems.Checked = true;
                chkArmor.Checked = true;
                chkWeapons.Checked = true;
                chkAmmo.Checked = true;
                chkWand.Checked = true;
                chkClothes.Checked = true;
                chkJewelry.Checked = true;
                chkBooks.Checked = true;
                chkKeys.Checked = true;
                chkComps.Checked = true;
                chkManastones.Checked = true;
                chkFood.Checked = true;
                chkGems.Checked = true;
                chkNotes.Checked = true;
                chkPacks.Checked = true;
                chkFletch.Checked = true;
                chkAlchemy.Checked = true;
                chkCook.Checked = true;
                chkSalvage.Checked = true;
                chkDecor.Checked = true;
                chkMisc.Checked = true;
            }
            else
            {
                chkAllWorld.Checked = false;
                chkAllHousing.Checked = false;
                chkAllSpawns.Checked = false;
                chkAllTown.Checked = false;
                chkAllItems.Checked = false;

                chkPortals.Checked = false;
                chkLifestones.Checked = false;
                chkDoors.Checked = false;
                chkWells.Checked = false;
                chkFountains.Checked = false;
                chkChests.Checked = false;
                chkTroves.Checked = false;

                chkCovenantCrystals.Checked = false;
                chkHouseChests.Checked = false;
                chkHooks.Checked = false;
                chkMansions.Checked = false;
                chkVillas.Checked = false;
                chkCottages.Checked = false;
                chkApartments.Checked = false;

                chkNPCs.Checked = false;
                chkMonSpawns.Checked = false;
                chkMonPTM.Checked = false;

                chkTownSigns.Checked = false;
                chkMerchantSigns.Checked = false;

                chkItems.Checked = false;
                chkArmor.Checked = false;
                chkWeapons.Checked = false;
                chkAmmo.Checked = false;
                chkWand.Checked = false;
                chkClothes.Checked = false;
                chkJewelry.Checked = false;
                chkBooks.Checked = false;
                chkKeys.Checked = false;
                chkComps.Checked = false;
                chkManastones.Checked = false;
                chkFood.Checked = false;
                chkGems.Checked = false;
                chkNotes.Checked = false;
                chkPacks.Checked = false;
                chkFletch.Checked = false;
                chkAlchemy.Checked = false;
                chkCook.Checked = false;
                chkSalvage.Checked = false;
                chkDecor.Checked = false;
                chkMisc.Checked = false;
            }
        }

        private void chkAllWorld_CheckedChanged(object sender, EventArgs e)
        {
            if (chkAllWorld.Checked)
            {
                chkPortals.Checked = true;
                chkLifestones.Checked = true;
                chkDoors.Checked = true;
                chkWells.Checked = true;
                chkFountains.Checked = true;
                chkChests.Checked = true;
                chkTroves.Checked = true;
            }
            else
            {
                chkPortals.Checked = false;
                chkLifestones.Checked = false;
                chkDoors.Checked = false;
                chkWells.Checked = false;
                chkFountains.Checked = false;
                chkChests.Checked = false;
                chkTroves.Checked = false;
            }
        }

        private void chkAllHousing_CheckedChanged(object sender, EventArgs e)
        {
            if (chkAllHousing.Checked)
            {
                chkCovenantCrystals.Checked = true;
                chkHouseChests.Checked = true;
                chkHooks.Checked = true;
                chkMansions.Checked = true;
                chkVillas.Checked = true;
                chkCottages.Checked = true;
                chkApartments.Checked = true;
            }
            else
            {
                chkCovenantCrystals.Checked = false;
                chkHouseChests.Checked = false;
                chkHooks.Checked = false;
                chkMansions.Checked = false;
                chkVillas.Checked = false;
                chkCottages.Checked = false;
                chkApartments.Checked = false;
            }
        }

        private void chkAllSpawns_CheckedChanged(object sender, EventArgs e)
        {
            if (chkAllSpawns.Checked)
            {
                chkNPCs.Checked = true;
                chkMonSpawns.Checked = true;
                chkMonPTM.Checked = true;
            }
            else
            {
                chkNPCs.Checked = false;
                chkMonSpawns.Checked = false;
                chkMonPTM.Checked = false;
            }
        }

        private void chkAllTown_CheckedChanged(object sender, EventArgs e)
        {
            if (chkAllTown.Checked)
            {
                chkTownSigns.Checked = true;
                chkMerchantSigns.Checked = true;
            }
            else
            {
                chkTownSigns.Checked = false;
                chkMerchantSigns.Checked = false;
            }
        }

        private void chkAllItems_CheckedChanged(object sender, EventArgs e)
        {
            if (chkAllItems.Checked)
            {
                chkItems.Checked = true;
                chkArmor.Checked = true;
                chkWeapons.Checked = true;
                chkAmmo.Checked = true;
                chkWand.Checked = true;
                chkClothes.Checked = true;
                chkJewelry.Checked = true;
                chkBooks.Checked = true;
                chkKeys.Checked = true;
                chkComps.Checked = true;
                chkManastones.Checked = true;
                chkFood.Checked = true;
                chkGems.Checked = true;
                chkNotes.Checked = true;
                chkPacks.Checked = true;
                chkFletch.Checked = true;
                chkAlchemy.Checked = true;
                chkCook.Checked = true;
                chkSalvage.Checked = true;
                chkDecor.Checked = true;
                chkMisc.Checked = true;
            }
            else
            {
                chkItems.Checked = false;
                chkArmor.Checked = false;
                chkWeapons.Checked = false;
                chkAmmo.Checked = false;
                chkWand.Checked = false;
                chkClothes.Checked = false;
                chkJewelry.Checked = false;
                chkBooks.Checked = false;
                chkKeys.Checked = false;
                chkComps.Checked = false;
                chkManastones.Checked = false;
                chkFood.Checked = false;
                chkGems.Checked = false;
                chkNotes.Checked = false;
                chkPacks.Checked = false;
                chkFletch.Checked = false;
                chkAlchemy.Checked = false;
                chkCook.Checked = false;
                chkSalvage.Checked = false;
                chkDecor.Checked = false;
                chkMisc.Checked = false;
            }
        }
        #endregion
        #region Menu Functions
        private void fileToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            OpenFileDialog oFile = new OpenFileDialog();
            oFile.DefaultExt = ".cap";
            oFile.Filter = "Capture files (*.cap;*.pcap)|*.cap;*.pcap|All files (*.*)|*.*";
            oFile.Title = "Select a Packet Capture File";

            if (oFile.ShowDialog() == DialogResult.OK)
            {
                path = oFile.FileName;
                lblTargetFile.Text = path;
                btnBeginScan.Enabled = true;
            }
        }

        private void folderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog oFolder = new FolderBrowserDialog();

            if (oFolder.ShowDialog() == DialogResult.OK)
            {
                dir = new DirectoryInfo(@oFolder.SelectedPath);
                openFolder = true;
                btnBeginScan.Enabled = true;
            }
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }
        #endregion

        private void btnBeginScan_Click(object sender, EventArgs e)
        {
            initialize();
            sw = new StreamWriter(@".\sqlout.txt");
            btnBeginScan.Enabled = false;

            if (chkNoDb.Checked == false)
            {
                //Cube: Make a connection to the database
                ConStr = "SERVER=127.0.0.1;PORT=3306;DATABASE=" + txtDBName.Text + ";UID=" + txtUser.Text + ";PWD=" + txtPass.Text + ";";
                dbConn = new MySqlConnection(ConStr);

                if (txtDBName.Text.Length > 0)
                {
                    //Cube: Now try opening it, but report error if it's a no go.
                    try
                    {
                        dbConn.Open();

                        //Cube: Get all the current values that we will need to increment
                        if (chkNPCs.Checked)
                        {
                            try { c_npc_palette_id = MySQLCommandEntry("SELECT MAX(ID) FROM npcs_vector_palettes", dbConn, c_npc_palette_id); }
                            catch (Exception ex) { c_npc_palette_id = 0; }
                            try { c_npc_texture_id = MySQLCommandEntry("SELECT MAX(ID) FROM npcs_vector_texturesvector", dbConn, c_npc_texture_id); }
                            catch (Exception ex) { c_npc_texture_id = 0; }
                            try { c_npc_model_id = MySQLCommandEntry("SELECT MAX(ID) FROM npcs_vector_models", dbConn, c_npc_model_id); }
                            catch (Exception ex) { c_npc_model_id = 0; }
                            try { c_npc_palette_mv = MySQLCommandEntry("SELECT MAX(ModelVector) FROM npcs_vector_palettes", dbConn, c_npc_palette_mv); }
                            catch (Exception ex) { c_npc_palette_mv = 0; }
                            try { c_npc_texture_mv = MySQLCommandEntry("SELECT MAX(ModelVector) FROM npcs_vector_textures", dbConn, c_npc_texture_mv); }
                            catch (Exception ex) { c_npc_texture_mv = 0; }
                            try { c_npc_model_mv = MySQLCommandEntry("SELECT MAX(ModelVector) FROM npcs_vector_models", dbConn, c_npc_model_mv); }
                            catch (Exception ex) { c_npc_model_mv = 0; }
                        }
                        //Items
                        if (chkItems.Checked)
                        {
                            try { c_item_palette_pk = MySQLCommandEntry("SELECT MAX(ID) FROM items_vector_palettes", dbConn, c_item_palette_pk); }
                            catch (Exception ex) { c_item_palette_pk = 0; }
                            c_item_texture_pk = MySQLCommandEntry("SELECT MAX(ID) FROM items_vector_textures", dbConn, c_item_texture_pk); 
                            //catch (Exception ex) { c_item_texture_pk = 0; }
                            try { c_item_model_pk = MySQLCommandEntry("SELECT MAX(ID) FROM items_vector_models", dbConn, c_item_model_pk); }
                            catch (Exception ex) { c_item_model_pk = 0; }
                            try { c_item_palette_mv = MySQLCommandEntry("SELECT MAX(Linker) FROM items_vector_palettes", dbConn, c_item_palette_mv); }
                            catch (Exception ex) { c_item_palette_mv = 0; }
                            try { c_item_texture_mv = MySQLCommandEntry("SELECT MAX(Linker) FROM items_vector_textures", dbConn, c_item_texture_mv); }
                            catch (Exception ex) { c_item_texture_mv = 0; }
                            try { c_item_model_mv = MySQLCommandEntry("SELECT MAX(Linker) FROM items_vector_models", dbConn, c_item_model_mv); }
                            catch (Exception ex) { c_item_model_mv = 0; }
                        }
                        //Monsters
                        if (chkMonPTM.Checked)
                        {
                            try { c_mon_palette_id = MySQLCommandEntry("SELECT MAX(ID) FROM monsters_vector_palettes", dbConn, c_mon_palette_id); }
                            catch (Exception ex) { c_mon_palette_id = 0; }
                            try { c_mon_texture_id = MySQLCommandEntry("SELECT MAX(ID) FROM monsters_vector_textures", dbConn, c_mon_texture_id); }
                            catch (Exception ex) { c_mon_texture_id = 0; }
                            try { c_mon_model_id = MySQLCommandEntry("SELECT MAX(ID) FROM monsters_vector_models", dbConn, c_mon_model_id); }
                            catch (Exception ex) { c_mon_model_id = 0; }
                            try { c_mon_palette_mv = MySQLCommandEntry("SELECT MAX(dwLinker) FROM monsters_vector_palettes", dbConn, c_mon_palette_mv); }
                            catch (Exception ex) { c_mon_palette_mv = 0; }
                            try { c_mon_texture_mv = MySQLCommandEntry("SELECT MAX(dwLinker) FROM monsters_vector_textures", dbConn, c_mon_texture_mv); }
                            catch (Exception ex) { c_mon_texture_mv = 0; }
                            try { c_mon_model_mv = MySQLCommandEntry("SELECT MAX(dwLinker) FROM monsters_vector_models", dbConn, c_mon_model_mv); }
                            catch (Exception ex) { c_mon_model_mv = 0; }
                        }
                    }
                    catch (MySqlException sqlex)
                    {
                        string message = sqlex.Message;
                        MessageBox.Show(message, "MySQL Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        //MessageBox.Show(sqlex.ToString());
                    }
                }
            }
            else
            {
                if (chkNPCs.Checked)
                {
                    c_npc_palette_id = 0;
                    c_npc_texture_id = 0;
                    c_npc_model_id = 0;
                    c_npc_palette_mv = 0;
                    c_npc_texture_mv = 0;
                    c_npc_model_mv = 0;
                }
                //Items
                if (chkItems.Checked)
                {
                    c_item_palette_pk = 0;
                    c_item_texture_pk = 0;
                    c_item_model_pk = 0;
                    c_item_palette_mv = 0;
                    c_item_texture_mv = 0;
                    c_item_model_mv = 0;
                }
                //Monsters
                if (chkMonPTM.Checked)
                {
                    c_mon_palette_id = 0;
                    c_mon_texture_id = 0;
                    c_mon_model_id = 0;
                    c_mon_palette_mv = 0;
                    c_mon_texture_mv = 0;
                    c_mon_model_mv = 0;
                }
            }

            if (openFolder)
            {
                FileStream fs;
                FileInfo[] files;               //Create an array of files using FileInfo object
                files = dir.GetFiles("*.*");    //Get all files for the current directory
                //getDirsFiles(dir);

                foreach (FileInfo file in files)//Iterate through the directory and print the files
                {
                    try
                    {
                        filesProcessed++;
                        labelFileNum.Text = "(" + filesProcessed + " of " + files.Length + ")";
                        labelFileNum.Refresh();

                        fs = File.OpenRead(@dir + "\\" + file.Name);
                        process(fs);
                        fs.Close();
                    }
                    catch (Exception ex) { }
                }
            }
            else
            {
                try
                {
                    FileStream fs;
                    fs = new FileStream(path, FileMode.Open);
                    process(fs);
                    fs.Close();
                }
                catch (Exception ex2) { }
            }

            sw.Close();     //Close the StreamWriter stream

            /* Calculate the total MySQL entries processed by summing the objects */
            #region Entries Processed
            processed =
                  iPortals
                + iLifestones
                + iDoors
                + iFountains
                + iWells
                + iChests
                + iTroves
                + iCovenants
                + iHooks
                + iHouseChests
                + iMansions
                + iVillas
                + iCottages
                + iApartments
                + iMonsterSpawns
                + iMonsterPTM
                + iNPCs
                + iTownSigns
                + iMerchantSigns
                + iItems
                + iArmor
                + iWeapons
                + iAmmo
                + iWands
                + iClothes
                + iJewelry
                + iBooks
                + iKeys
                + iComps
                + iManastones
                + iFood
                + iGems
                + iNotes
                + iPack
                + iFletch
                + iAlchemy
                + iCook
                + iSalvage
                + iDecor
                + iMisc;
            #endregion

            MessageBox.Show("Total Create Messages: " + messageCount +
                "\n\rEntries Processed: " + processed +
                ".\n\n\rThe next NPC Palette ID is: " + c_npc_palette_id +
                ".\n\rThe next NPC Texture ID is:" + c_npc_texture_id +
                ".", "Results", MessageBoxButtons.OK, MessageBoxIcon.Information);

            if (chkNoDb.Checked == false)
                dbConn.Close(); //Cube: Close the DB connection

            //Cube: Launch Notepad with the sqlout file
            string ApplicationPath = "notepad.exe";
            string ApplicationArguments = "sqlout.txt";
            Process ProcessObj = new Process();

            ProcessObj.StartInfo.FileName = ApplicationPath;
            ProcessObj.StartInfo.Arguments = ApplicationArguments;

            ProcessObj.StartInfo.UseShellExecute = false;
            ProcessObj.StartInfo.RedirectStandardOutput = true;

            ProcessObj.Start(); //Cube: Start the process

            //Cube:  Don't need this
            // Wait that the process exits
            //ProcessObj.WaitForExit();
            //ProcessObj.Close();

            /* Populate the "Objects Found" labels */
            #region Objects Found Labels
            lblPortalTotal.Text = iPortals.ToString();
            lblLifestonesTotal.Text = iLifestones.ToString();
            lblDoorsTotal.Text = iDoors.ToString();
            lblFountainsTotal.Text = iFountains.ToString();
            lblWellsTotal.Text = iWells.ToString();
            lblChestsTotal.Text = iChests.ToString();
            lblTrovesTotal.Text = iTroves.ToString();
            lblCovenantTotal.Text = iCovenants.ToString();
            lblStorageTotal.Text = iHouseChests.ToString();
            lblHooksTotal.Text = iHooks.ToString();
            lblMansionsTotal.Text = iMansions.ToString();
            lblVillasTotal.Text = iVillas.ToString();
            lblCottagesTotal.Text = iCottages.ToString();
            lblApartmentsTotal.Text = iApartments.ToString();
            lblSpawnsTotal.Text = iMonsterSpawns.ToString();
            lblMonPTMTotal.Text = iMonsterPTM.ToString();
            lblNPCsTotal.Text = iNPCs.ToString();
            lblTownSignsTotal.Text = iTownSigns.ToString();
            lblMerchantSignsTotal.Text = iMerchantSigns.ToString();
            lblItemsTotal.Text = iItems.ToString();
            lblArmorTotal.Text = iArmor.ToString();
            lblWeaponsTotal.Text = iWeapons.ToString();
            lblAmmoTotal.Text = iAmmo.ToString();
            lblWandsTotal.Text = iWands.ToString();
            lblClothesTotal.Text = iClothes.ToString();
            lblJewelryTotal.Text = iJewelry.ToString();
            lblBooksTotal.Text = iBooks.ToString();
            lblKeysTotal.Text = iKeys.ToString();
            lblCompsTotal.Text = iComps.ToString();
            lblManastonesTotal.Text = iManastones.ToString();
            lblFoodTotal.Text = iFood.ToString();
            lblGemsTotal.Text = iGems.ToString();
            lblNotesTotal.Text = iNotes.ToString();
            lblPackTotal.Text = iPack.ToString();
            lblFletchTotal.Text = iFletch.ToString();
            lblAlchemyTotal.Text = iAlchemy.ToString();
            lblCookTotal.Text = iCook.ToString();
            lblSalvageTotal.Text = iSalvage.ToString();
            lblDecorTotal.Text = iDecor.ToString();
            lblMiscTotal.Text = iMisc.ToString();
            #endregion

            btnBeginScan.Enabled = true;
            openFolder = false;
        }

        /*** The function used to process an individual file ***/
        public void process(FileStream fs)
        {
            try
            {
                byte[] data = new byte[fs.Length];
                fs.Read(data, 0, data.Length);
                int i = 0;

                pbProgress.Maximum = data.Length;
                pbProgress.Value = 0;

                while (i <= (data.Length - 4))
                {
                    int data2 = BitConverter.ToInt32(data, i);

                    pbProgress.Increment(1);

                    //Look for a 0x0000F745 (the 4 bytes which were converted by BitConverter into 'data2')
                    if (data2 == 0x0000F745)
                    {
                        messageCount++;
                        int readIndex = 0;
                        int writeIndex = 0;
                        int extraData;  //Used for data that is apparently supplemental to the object.

                        //TODO:  Fix.  Arbitrarily large length for both; find better solution later.
                        byte[] pos = new byte[data.Length];
                        byte[] res = new byte[data.Length];

                        StringBuilder sb = new StringBuilder("(");

                        //String builders for Monster spawns
                        StringBuilder sb_spawns = new StringBuilder("INSERT INTO monsters (Landblock, Position_X, Position_Y, Position_Z, Orientation_W, Orientation_X, Orientation_Y, Orientation_Z, strName) VALUES(");
                        //This updates the NPC tables
                        StringBuilder sb_npcs = new StringBuilder("INSERT INTO npcs (Landblock, Position_X, Position_Y, Position_Z, Orientation_W, Orientation_X, Orientation_Y, Orientation_Z, npc_id, Name, Gender, wModelNum) VALUES(");
                        StringBuilder sb_modeldata = new StringBuilder("INSERT INTO npcs_models (Modelnum, ModelName, PaletteVector, ucPaletteChangeCount, TextureVector, ucTextureChangeCount, ModelVector, ucModelChangeCount, wModelID, wIconID, AnimationStrip, unkDataEntry, unkDataEntry2, dwModel, wPaletteCode) VALUES(");
                        //This updates the item table
                        StringBuilder sb_itemmodeldata = new StringBuilder("INSERT INTO `items_templates` (`Name`,`bPalette`,`bTexture`,`bModel`,`dwFlags1`,`wUnknown_1`,`wModel`,`wIcon`,`dwModelNumber`,`dwLinker`,`dwFlags2`,`dwObjectFlags1`,`dwObjectFlags2`,`dwValue`,`wBurden`) VALUES (");
                        StringBuilder sb_itemmodeldata_armor = new StringBuilder("INSERT INTO `item_model_data` (`Name`,`bPalette`,`bTexture`,`bModel`,`dwFlags1`,`wUnknown_1`,`wModel`,`wIcon`,`dwModelNumber`,`dwUnknown_v2`,`dwLinker`,`dwFlags2`,`dwObjectFlags1`,`dwObjectFlags2`,`dwValue`,`dwEquipPoss`,`dwCoverage`,`wBurden`,`wHooks`) VALUES (");
                        StringBuilder sb_worldobj2modeldata = new StringBuilder("INSERT INTO `worldobjects_templates` (`Name`,`bPalette`,`bTexture`,`bModel`,`dwFlags1`,`wModel`,`wIcon`,`dwModelNumber`,`dwLinker`,`dwFlags2`,`dwObjectFlags1`,`dwObjectFlags2`) VALUES (");
                        //TODO:  Complete this line.  world_objects2.dwModelNumber needs to be an int vs Varchar.
                        StringBuilder sb_worldobjects2 = new StringBuilder("INSERT INTO world_objects2 (Name,dwModelNumber,Landblock, Position_X, Position_Y, Position_Z, Orientation_W, Orientation_X, Orientation_Y, Orientation_Z,wModel,wIcon) VALUES("); ;
                        StringBuilder sb_monsters_templates = new StringBuilder("INSERT INTO `monsters_templates` (`Name`,`bPalette`,`bTexture`,`bModel`,`dwFlags1`,`wPortalMode`,`wUnknown_1`,`wModel`,`wIcon`,`AnimConfig`,`SoundSet`,`dwUnknown_Blue`,`dwModelNumber`,`flScale`,`dwUnknown_LightGrey`,`dwUnknown_v2`,`dwUnknown_v6`,`dwUnkCount`,`unknownBytes`,`dwLinker`,`dwObjectFlags1`,`dwObjectFlags2`) VALUES (");
                        StringBuilder sb_wo2LB = new StringBuilder();
                        StringBuilder sb_wo2x = new StringBuilder();
                        StringBuilder sb_wo2y = new StringBuilder();
                        StringBuilder sb_wo2z = new StringBuilder();
                        StringBuilder sb_wo2wQuat = new StringBuilder();
                        StringBuilder sb_wo2xQuat = new StringBuilder();
                        StringBuilder sb_wo2yQuat = new StringBuilder();
                        StringBuilder sb_wo2zQuat = new StringBuilder();

                        StringBuilder sb_m_Anim = new StringBuilder();
                        StringBuilder sb_m_sound = new StringBuilder();

                        fs.Position = i;

                        //First DWORD, which is F745 (Create Object) in little endian (0x45F70000)
                        fs.Read(pos, readIndex, 4);
                        res.SetValue(pos[3], 0);
                        res.SetValue(pos[2], 1);
                        res.SetValue(pos[1], 2);
                        res.SetValue(pos[0], 3);

                        //ObjectID object (object ID)
                        fs.Read(pos, readIndex, 4);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        /** Model Data **/
                        #region Model Data
                        //The ModelData structure defines an object's visual appearance.

                        fs.Read(pos, readIndex, 4);
                        byte byteEleven = pos[readIndex];
                        res.SetValue(pos[readIndex++], writeIndex++);    //BYTE eleven (always 0x11)
                        byte paletteCount = pos[readIndex];
                        res.SetValue(pos[readIndex++], writeIndex++);    //BYTE paletteCount (the number of palettes associated with this object)
                        byte textureCount = pos[readIndex];
                        res.SetValue(pos[readIndex++], writeIndex++);    //BYTE textureCount (the number of textures associated with this object)
                        byte modelCount = pos[readIndex];
                        res.SetValue(pos[readIndex++], writeIndex++);    //BYTE modelCount (the number of models associated with this object)

                        //It is possible for 0x0000F745 to be in data; error check here, as byte seems generally constant.
                        if ((byteEleven != 0x11) && (byteEleven != 0xFF))
                            goto Skip;

                        if ((paletteCount & 0xFF) == 0xFF)
                        {
                            //PackedDWORD palette (palette ResourceID (minus 0x04000000))                          
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            int num = int.Parse(BitConverter.ToString(res, writeIndex - 4, 4), System.Globalization.NumberStyles.AllowHexSpecifier);
                        }

                        int totalModelCount = 0;

                        //palettes
                        #region Palettes
                        if (paletteCount != 0)
                        {
                            //Advance the model once for each palette iteration
                            c_npc_palette_mv++;
                            c_item_palette_mv++;
                            c_mon_palette_mv++;

                            for (int j = 0; j < paletteCount; j++)  //vector of length paletteCount
                            {
                                //Cube: We pull the palette info out for NPCs (and other things) here.

                                //Advance the PK once per iteration
                                c_npc_palette_id++;
                                c_item_palette_pk++;
                                c_mon_palette_id++;

                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex + 1], writeIndex++);
                                res.SetValue(pos[readIndex + 0], writeIndex++);

                                string data3 = (BitConverter.ToString(res, writeIndex - 2, 2));
                                data3 = data3.Replace("-", "");
                                //Believed to be a reference to the human palette; adds an extra DWORD to the palette data.
                                //Only appears on human objects or items that affect human appearance (i.e., armor).
                                if (data3 == "007E" || data3 == "0BEF")
                                {
                                    wPaletteCode = data3;

                                    fs.Read(pos, readIndex, 2);
                                    res.SetValue(pos[readIndex + 1], writeIndex++);
                                    res.SetValue(pos[readIndex + 0], writeIndex++);
                                    sNewPalette[j] = "(" + c_npc_palette_id + "," + c_npc_palette_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 2, 2)) + "',";
                                    Item_sNewPalette[j] = "(" + c_npc_palette_id + "," + c_npc_palette_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 2, 2)) + "',";
                                    m_NewPalette[j] = "(" + c_npc_palette_id + "," + c_npc_palette_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 2, 2)) + "',";
                                }

                                //WORD newPalette (the Portal.dat ID of the palette from which we are drawing entries) 

                                sNewPalette[j] = "(" + c_npc_palette_id + "," + c_npc_palette_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 2, 2)) + "',";
                                Item_sNewPalette[j] = "(" + c_item_palette_pk + "," + c_item_palette_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 2, 2)) + "',";
                                m_NewPalette[j] = "(" + c_mon_palette_id + "," + c_mon_palette_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 2, 2)) + "',";

                                fs.Read(pos, readIndex, 2);

                                //BYTE offset (the number of palette entries to skip)
                                res.SetValue(pos[readIndex++], writeIndex++);
                                sPaletteOffset[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                Item_sPaletteOffset[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                m_PaletteOffset[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";

                                res.SetValue(pos[readIndex++], writeIndex++);

                                //Cube:  Test for last line, if it is last line add a ; instead of ,
                                if (j == paletteCount - 1)
                                {
                                    sPaletteLength[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                    Item_sPaletteLength[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                    m_PaletteLength[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                }
                                else
                                {
                                    sPaletteLength[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                    Item_sPaletteLength[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                    m_PaletteLength[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                }

                                totalModelCount += 4;
                            }

                            //WORD (Palette padding)
                            //fs.Read(pos, readIndex, 2);
                            //res.SetValue(pos[1], 0);
                            //res.SetValue(pos[0], 1);
                            //res.SetValue(pos[readIndex + 1], writeIndex++);
                            //res.SetValue(pos[readIndex + 0], writeIndex++);

                            totalModelCount += 2;
                            //Palette Code
                            //string blah = (BitConverter.ToString(res, writeIndex - 2, 2));
                            //wPaletteCode = blah.Replace("-", "");
                            //sb.Append(BitConverter.ToString(res, writeIndex - 2, 2));
                        }
                        #endregion

                        //textures
                        #region Textures
                        if (textureCount != 0)
                        {
                            //Advance the next count for the textures model vector
                            c_npc_texture_mv = c_npc_palette_mv;


                            //If the palette count is zero, add one to the ModelVector to use.
                            if (paletteCount == 0)
                            {
                                c_item_texture_mv++;
                                c_mon_texture_mv++;

                            }
                            //Otherwise set the modelvector to the same as the palette vector.
                            else
                            {
                                c_item_texture_mv = c_item_palette_mv;
                                c_mon_texture_mv = c_mon_palette_mv;
                            }

                            for (int j = 0; j < textureCount; j++)  //vector of length textureCount
                            {
                                //Advance the count once per iteration
                                c_npc_texture_id++;
                                c_item_texture_pk++;
                                c_mon_texture_id++;

                                extraData = BitConverter.ToInt32(res, writeIndex - 4);
                                if ((extraData & 0x3C9A0500) == 0x3C9A0500)
                                {
                                    goto Skip;
                                    for (int l = 0; l < 78; l++)    //78 bytes, prefixed by 0x00059A3C
                                    {
                                        fs.Read(pos, readIndex, 1);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                    }
                                }

                                fs.Read(pos, readIndex, 1);
                                //BYTE index (the index of the model in which we are replacing the texture)
                                res.SetValue(pos[readIndex++], writeIndex++);
                                sTModelIndex[j] = "(" + c_npc_texture_id + "," + c_npc_texture_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                Item_sTModelIndex[j] = "(" + c_item_texture_pk + "," + c_item_texture_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                m_TModelIndex[j] = "(" + c_mon_texture_id + "," + c_mon_texture_mv + "," + (j + 1) + "," + "'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                //PackedDWORD new? (texture ResourceID (minus 0x05000000))
                                for (int k = 0; k < 4; k++)
                                {
                                    extraData = BitConverter.ToInt32(res, writeIndex - 4);
                                    if ((extraData & 0x3C9A0500) == 0x3C9A0500)
                                    {
                                        goto Skip;
                                        for (int l = 0; l < 78; l++)    //78 bytes, prefixed by 0x00059A3C
                                        {
                                            fs.Read(pos, readIndex, 1);
                                            res.SetValue(pos[readIndex++], writeIndex++);
                                        }
                                    }
                                }
                                //Cube:  Need to do some funky stuff here to get this to work.
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex + 3], writeIndex++);
                                sOldTexture2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));
                                Item_sOldTexture2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));
                                m_OldTexture2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));

                                res.SetValue(pos[readIndex + 2], writeIndex++);
                                if (j == textureCount - 1)
                                {
                                    sOldTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                    Item_sOldTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                    m_OldTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                }
                                else
                                {
                                    sOldTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                    Item_sOldTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                    m_OldTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                }

                                res.SetValue(pos[readIndex + 1], writeIndex++);
                                sNewTexture2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));
                                Item_sNewTexture2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));
                                m_NewTexture2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));

                                res.SetValue(pos[readIndex + 0], writeIndex++);
                                sNewTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                Item_sNewTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                m_NewTexture1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";

                                totalModelCount += 5;
                            }
                        }
                        #endregion

                        //models
                        #region Models
                        if (modelCount != 0)
                        {
                            c_npc_model_mv = c_npc_palette_mv;

                            if (paletteCount == 0)
                            {
                                c_item_palette_mv += 1;
                                c_item_model_mv++;
                                c_mon_model_mv++;
                            }
                            else
                            {
                                c_item_model_mv = c_item_palette_mv;
                                c_mon_model_mv = c_mon_palette_mv;
                            }

                            for (int j = 0; j < modelCount; j++) //vector of length modelCount
                            {
                                c_npc_model_id++;
                                c_item_model_pk++;
                                c_mon_model_id++;

                                extraData = BitConverter.ToInt32(res, writeIndex - 4);
                                if ((extraData & 0x3C9A0500) == 0x3C9A0500)
                                {
                                    goto Skip;
                                    for (int l = 0; l < 78; l++)    //78 bytes, prefixed by 0x00059A3C
                                    {
                                        fs.Read(pos, readIndex, 1);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                    }
                                }

                                //BYTE readIndex (the index of the model)
                                fs.Read(pos, readIndex, 1);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                sMModelIndex[j] = "(" + c_npc_model_id + "," + c_npc_model_mv + "," + (j + 1) + ",'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                Item_sMModelIndex[j] = "(" + c_item_model_pk + "," + c_item_model_mv + "," + (j + 1) + ",'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";
                                m_MModelIndex[j] = "(" + c_item_model_pk + "," + c_item_model_mv + "," + (j + 1) + ",'" + (BitConverter.ToString(res, writeIndex - 1, 1)) + "',";

                                //WORD newModel (the portal.dat ID of the new type 1 model)
                                for (int k = 0; k < 2; k++)
                                {
                                    extraData = BitConverter.ToInt32(res, writeIndex - 4);
                                    if ((extraData & 0x3C9A0500) == 0x3C9A0500)
                                    {
                                        goto Skip;
                                        for (int l = 0; l < 78; l++)    //78 bytes, prefixed by 0x00059A3C
                                        {
                                            fs.Read(pos, readIndex, 1);
                                            res.SetValue(pos[readIndex++], writeIndex++);
                                        }
                                    }
                                }
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex + 1], writeIndex++);
                                sNewModel2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));
                                Item_sNewModel2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));
                                m_NewModel2[j] = "'" + (BitConverter.ToString(res, writeIndex - 1, 1));

                                res.SetValue(pos[readIndex + 0], writeIndex++);
                                if (j == modelCount - 1)
                                {
                                    sNewModel1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                    Item_sNewModel1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                    m_NewModel1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "');";
                                }
                                else
                                {
                                    sNewModel1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                    Item_sNewModel1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                    m_NewModel1[j] = (BitConverter.ToString(res, writeIndex - 1, 1)) + "'),";
                                }

                                totalModelCount += 3;
                            }
                        }
                        #endregion

                        while (totalModelCount % 4 != 0)    //Model Data padding
                        {
                            fs.Read(pos, readIndex, 1);
                            readIndex++;
                            totalModelCount++;
                        }
                        #endregion


                        /** Physics Data **/
                        #region Physics Data
                        //The PhysicsData structure defines an object's physical behavior.

                        //DWORD flags (physics data flags)    
                        string[] item_flags1 = new string[4];
                        fs.Read(pos, readIndex, 4);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        item_flags1[3] = BitConverter.ToString(res, writeIndex - 1, 1);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        item_flags1[2] = BitConverter.ToString(res, writeIndex - 1, 1);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        item_flags1[1] = BitConverter.ToString(res, writeIndex - 1, 1);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        item_flags1[0] = BitConverter.ToString(res, writeIndex - 1, 1);

                        int physicsFlags = BitConverter.ToInt32(res, writeIndex - 4);

                        string s_dwFlags1 = item_flags1[0] + item_flags1[1] + item_flags1[2] + item_flags1[3];

                        fs.Read(pos, readIndex, 4);
                        //DWORD unknown
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        wPortalMode = (ushort)BitConverter.ToInt16(res, writeIndex - 2);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        unknown1 = (ushort)BitConverter.ToInt16(res, writeIndex - 2);

                        //Flag & 0x00010000
                        if ((physicsFlags & 0x00010000) == 0x00010000)
                        {
                            extraData = BitConverter.ToInt32(res, writeIndex - 4);
                            if ((extraData & 0x3C9A0500) == 0x3C9A0500)
                            {
                                goto Skip;
                                for (int l = 0; l < 78; l++)    //78 bytes, prefixed by 0x00059A3C
                                {
                                    fs.Read(pos, readIndex, 1);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                }
                            }

                            //DWORD byteCount (the number of BYTEs that follow)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            byteCount = BitConverter.ToInt32(res, writeIndex - 4);

                            if (byteCount != 0)
                            {
                                for (int j = 0; j < byteCount; j++) //vector of length byteCount
                                {
                                    //BYTE byte
                                    fs.Read(pos, readIndex, 1);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                };
                                unknownBytes = BitConverter.ToString(res, writeIndex - byteCount, byteCount);
                                unknownBytes = unknownBytes.Replace("-", "");    //By default, BitConverter converts the bytes to a string with dashes (i.e., 12-34-56-78); sb.Replace is used to remove these.

                                //DWORD unknown10000
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                        }

                        //Flag & 0x00020000
                        if ((physicsFlags & 0x00020000) == 0x00020000)
                        {
                            //DWORD unknown20000
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        //Flag & 0x00008000
                        #region Position
                        if ((physicsFlags & 0x00008000) == 0x00008000)
                        {
                            //Position0position (object position)
                            /* Position0
                             * A position structure with an implied flags value of 0. */

                            //DWORD landcell (the landcell in which the object is located)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex + 3], writeIndex++);
                            res.SetValue(pos[readIndex + 2], writeIndex++);
                            res.SetValue(pos[readIndex + 1], writeIndex++);
                            res.SetValue(pos[readIndex + 0], writeIndex++);
                            sb.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb.Replace("-", "");    //By default, StringBuilder converts the bytes to a string with dashes (i.e., 00-00-00-00); sb.Replace is used to remove these.
                            //Cube:  Added this for spawns
                            sb_spawns.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_spawns.Replace("-", "");
                            //Cube:  Added for NPCs
                            sb_npcs.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_npcs.Replace("-", "");
                            sb_wo2LB.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "'");
                            sb_wo2LB.Replace("-", "");
                            wo2lb = sb_wo2LB.ToString();
                            readIndex += 4;

                            //displacement (a vector describing the object's position within the landblock containing the landcell)
   //                         fs.Read(pos, readIndex, 12);

                            //float x
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex + 3], writeIndex++);
                            res.SetValue(pos[readIndex + 2], writeIndex++);
                            res.SetValue(pos[readIndex + 1], writeIndex++);
                            res.SetValue(pos[readIndex + 0], writeIndex++);
                            sb.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb.Replace("-", "");    //By default, StringBuilder converts the bytes to a string with dashes (i.e., 00-00-00-00); sb.Replace is used to remove these.
                            //Cube:  Added this for spawns
                            sb_spawns.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_spawns.Replace("-", "");
                            //Cube:  Added for NPCs
                            sb_npcs.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_npcs.Replace("-", "");
                            sb_wo2x.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "'");
                            sb_wo2x.Replace("-", "");
                            wo2x = sb_wo2x.ToString();
                            readIndex += 4;
                            //float y
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex + 3], writeIndex++);
                            res.SetValue(pos[readIndex + 2], writeIndex++);
                            res.SetValue(pos[readIndex + 1], writeIndex++);
                            res.SetValue(pos[readIndex + 0], writeIndex++);
                            sb.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb.Replace("-", "");    //By default, StringBuilder converts the bytes to a string with dashes (i.e., 00-00-00-00); sb.Replace is used to remove these.
                            //Cube:  Added this for spawns
                            sb_spawns.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_spawns.Replace("-", "");
                            //Cube:  Added for NPCs
                            sb_npcs.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_npcs.Replace("-", "");
                            sb_wo2y.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "'");
                            sb_wo2y.Replace("-", "");
                            wo2y = sb_wo2y.ToString();
                            readIndex += 4;
                            //float z
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex + 3], writeIndex++);
                            res.SetValue(pos[readIndex + 2], writeIndex++);
                            res.SetValue(pos[readIndex + 1], writeIndex++);
                            res.SetValue(pos[readIndex + 0], writeIndex++);
                            sb.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb.Replace("-", "");    //By default, StringBuilder converts the bytes to a string with dashes (i.e., 00-00-00-00); sb.Replace is used to remove these.
                            //Cube:  Added this for spawns
                            sb_spawns.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_spawns.Replace("-", "");
                            //Cube:  Added for NPCs
                            sb_npcs.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_npcs.Replace("-", "");
                            sb_wo2z.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "'");
                            sb_wo2z.Replace("-", "");
                            wo2z = sb_wo2z.ToString();
                            readIndex += 4;

                            //float wQuat
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex + 3], writeIndex++);
                            res.SetValue(pos[readIndex + 2], writeIndex++);
                            res.SetValue(pos[readIndex + 1], writeIndex++);
                            res.SetValue(pos[readIndex + 0], writeIndex++);
                            sb.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb.Replace("-", "");    //By default, StringBuilder converts the bytes to a string with dashes (i.e., 00-00-00-00); sb.Replace is used to remove these.
                            //Cube:  Added this for spawns
                            sb_spawns.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_spawns.Replace("-", "");
                            //Cube:  Added for NPCs
                            sb_npcs.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_npcs.Replace("-", "");
                            sb_wo2wQuat.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "'");
                            sb_wo2wQuat.Replace("-", "");
                            wo2wQuat = sb_wo2wQuat.ToString();
                            readIndex += 4;
                            //float xQuat
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex + 3], writeIndex++);
                            res.SetValue(pos[readIndex + 2], writeIndex++);
                            res.SetValue(pos[readIndex + 1], writeIndex++);
                            res.SetValue(pos[readIndex + 0], writeIndex++);
                            sb.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb.Replace("-", "");    //By default, StringBuilder converts the bytes to a string with dashes (i.e., 00-00-00-00); sb.Replace is used to remove these.
                            //Cube:  Added this for spawns
                            sb_spawns.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_spawns.Replace("-", "");
                            //Cube:  Added for NPCs
                            sb_npcs.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_npcs.Replace("-", "");
                            sb_wo2xQuat.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "'");
                            sb_wo2xQuat.Replace("-", "");
                            wo2xQuat = sb_wo2xQuat.ToString();
                            readIndex += 4;
                            //float yQuat
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex + 3], writeIndex++);
                            res.SetValue(pos[readIndex + 2], writeIndex++);
                            res.SetValue(pos[readIndex + 1], writeIndex++);
                            res.SetValue(pos[readIndex + 0], writeIndex++);
                            sb.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb.Replace("-", "");    //By default, StringBuilder converts the bytes to a string with dashes (i.e., 00-00-00-00); sb.Replace is used to remove these.
                            //Cube:  Added this for spawns
                            sb_spawns.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_spawns.Replace("-", "");
                            //Cube:  Added for NPCs
                            sb_npcs.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_npcs.Replace("-", "");
                            sb_wo2yQuat.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "'");
                            sb_wo2yQuat.Replace("-", "");
                            wo2yQuat = sb_wo2yQuat.ToString();
                            readIndex += 4;
                            //float zQuat
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex + 3], writeIndex++);
                            res.SetValue(pos[readIndex + 2], writeIndex++);
                            res.SetValue(pos[readIndex + 1], writeIndex++);
                            res.SetValue(pos[readIndex + 0], writeIndex++);
                            sb.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb.Replace("-", "");    //By default, StringBuilder converts the bytes to a string with dashes (i.e., 00-00-00-00); sb.Replace is used to remove these.
                            //Cube:  Added this for spawns
                            sb_spawns.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_spawns.Replace("-", "");
                            //Cube:  Added for NPCs
                            sb_npcs.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "',");
                            sb_npcs.Replace("-", "");
                            sb_wo2zQuat.Append("'" + BitConverter.ToString(res, writeIndex - 4, 4) + "'");
                            sb_wo2zQuat.Replace("-", "");
                            wo2zQuat = sb_wo2zQuat.ToString();
                            readIndex += 4;
                        }
                        #endregion

                        //Flag & 0x00000002
                        if ((physicsFlags & 0x00000002) == 0x00000002)
                        {
                            //ResourceID animations (animation set ResourceID)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            int animationID = BitConverter.ToInt32(res, writeIndex - 4);
                            sb.Append((animationID & 0xF6FFFFFF) + ",");    //exclude 0x09000000 from animationID
                            sb_m_Anim.Append("'" + BitConverter.ToString(res, writeIndex - 3, 1) + BitConverter.ToString(res, writeIndex - 4, 1) + "'");
                            sb_m_Anim.Replace("-", "");
                            m_Anim = sb_m_Anim.ToString();
                        }

                        //Flag & 0x00000800
                        if ((physicsFlags & 0x00000800) == 0x00000800)
                        {
                            //ResourceID sounds (sound set ResourceID)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            int soundID = BitConverter.ToInt32(res, writeIndex - 4);
                            sb.Append((soundID & 0xDFFFFFFF) + ",");        //exclude 0x20000000 from soundID
                            //sb_m_sound.Append((soundID & 0xDFFFFFFF) + ","); 
                            sb_m_sound.Append("'" + BitConverter.ToString(res, writeIndex - 3, 1) + BitConverter.ToString(res, writeIndex - 4, 1) + "'");
                            sb_m_sound.Replace("-", "");
                            m_Sound = sb_m_sound.ToString();
                        }

                        //Flag & 0x00001000
                        if ((physicsFlags & 0x00001000) == 0x00001000)
                        {
                            //ResourceID unknown1000 (unknown ResourceID)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            int resourceID = BitConverter.ToInt32(res, writeIndex - 4);
                            //sb.Append((resourceID) + ",");

                            m_res = BitConverter.ToInt32(res, writeIndex - 1);
                        }

                        //Flag & 0x00000001
                        if ((physicsFlags & 0x00000001) == 0x00000001)
                        {
                            string[] item_model = new string[4];
                            //ResourceID model (model ResourceID)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_model[3] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_model[2] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_model[1] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_model[0] = BitConverter.ToString(res, writeIndex - 1, 1);

                            int modelID = BitConverter.ToInt32(res, writeIndex - 4);
                            sb.Append((modelID & 0xFDFFFFFF) + ",");        //exclude 0x02000000 from modelID

                            //dwModelNumber = (modelID & 0xFDFFFFFF) + "";
                            //wModel = (modelID & 0xFDFFFFFF) + "";
                            wModel = item_model[2] + item_model[3];
                        }


                        //Flag & 0x00000020
                        if ((physicsFlags & 0x00000020) == 0x00000020)
                        {
                            //ObjectID equipper (the creature equipping this object)
                            /* EquipMask equipperSlot (the slot in which this object is equipped)
                             * EquipMask (DWORD)
                             * The EquipMask value describes the equipment slots an item uses. */
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        //Flag & 0x00000040
                        if ((physicsFlags & 0x00000040) == 0x00000040)
                        {
                            //DWORD equippedCount (the number of items equipped by this creature)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            int equippedCount = BitConverter.ToInt32(res, writeIndex - 4);

                            if (equippedCount != 0)
                            {
                                //Cube: This should be equipped count, was texturecount
                                for (int j = 0; j <= equippedCount; j++)  //vector of length equippedCount
                                {
                                    /* EquipMask (DWORD)
                                     * The EquipMask value describes the equipment slots an item uses. */
                                    fs.Read(pos, readIndex, 4);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                }
                            }
                        }

                        //Flag & 0x00000080
                        if ((physicsFlags & 0x00000080) == 0x00000080)
                        {
                            //float scale (the size of this object)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            scale = BitConverter.ToSingle(res, writeIndex - 4);
                        }

                        //Flag & 0x00000100
                        if ((physicsFlags & 0x00000100) == 0x00000100)
                        {
                            //DWORD unknown100
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        //Flag & 0x00000200
                        if ((physicsFlags & 0x00000200) == 0x00000200)
                        {
                            //DWORD unknown200
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        //Flag & 0x00040000
                        if ((physicsFlags & 0x00040000) == 0x00040000)
                        {
                            //float unknown40000
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            unknown100 = BitConverter.ToInt32(res, writeIndex - 4);
                        }

                        //Flag & 0x00000004
                        if ((physicsFlags & 0x00000004) == 0x00000004)
                        {
                            fs.Read(pos, readIndex, 12);

                            //float dx (velocity vector x component)
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);

                            //float dy (velocity vector y component)
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);

                            //float dz (velocity vector z component)
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        //Flag & 0x00000008
                        if ((physicsFlags & 0x00000008) == 0x00000008)
                        {
                            fs.Read(pos, readIndex, 12);

                            //float unknown8_1
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);

                            //float unknown8_2
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);

                            //float unknown8_3
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        //Flag & 0x00000010
                        if ((physicsFlags & 0x00000010) == 0x00000010)
                        {
                            fs.Read(pos, readIndex, 12);

                            //float rx (rotation vector x component)
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);

                            //float ry (rotation vector y component)
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);

                            //float rz (rotation vector z component)
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        //Flag & 0x00002000
                        if ((physicsFlags & 0x00002000) == 0x00002000)
                        {
                            //DWORD unknown2000
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        //Flag & 0x00004000
                        if ((physicsFlags & 0x00004000) == 0x00004000)
                        {
                            //DWORD unknown4000
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                        }

                        fs.Read(pos, readIndex, 20);

                        //WORD unknown1
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown2
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown3
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown4
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown5
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown6
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown7
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown8
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown9
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);

                        //WORD unknown10
                        res.SetValue(pos[readIndex++], writeIndex++);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        #endregion


                        /** Game Data **/
                        #region Game Data
                        //The GameData structure defines an object's game behavior.

                        #region Flags1
                        //DWORD flags1 (game data flags)
                        string[] item_flags2 = new string[4];
                        fs.Read(pos, readIndex, 4);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        item_flags2[3] = BitConverter.ToString(res, writeIndex - 1, 1);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        item_flags2[2] = BitConverter.ToString(res, writeIndex - 1, 1);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        item_flags2[1] = BitConverter.ToString(res, writeIndex - 1, 1);
                        res.SetValue(pos[readIndex++], writeIndex++);
                        item_flags2[0] = BitConverter.ToString(res, writeIndex - 1, 1);

                        int gameDataFlags1 = BitConverter.ToInt32(res, writeIndex - 4);

                        string s_dwFlags2 = item_flags2[0] + item_flags2[1] + item_flags2[2] + item_flags2[3];
                        #endregion
                        if (gameDataFlags1 != 0x0000F745)   // Apparently some objects do not contain game data
                        {
                            #region Name
                            //String name (object name)
                            fs.Read(pos, readIndex, 2);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            ushort stringCount = (ushort)BitConverter.ToInt16(res, writeIndex - 2);

                            fs.Read(pos, readIndex, stringCount);
                            for (int j = 0; j <= stringCount; j++)  //vector of length stringCount
                            {
                                res.SetValue(pos[readIndex++], writeIndex++);
                                if (j == stringCount)
                                {
                                    System.Text.Encoding ascii = System.Text.Encoding.ASCII;    //define ASCII format
                                    string aString = ascii.GetString(res, writeIndex - stringCount - 1, stringCount);
                                    aString = aString.Replace("'", "");
                                    sb.Append("'" + aString + "',"); //write our hexadecimal as an ASCII string
                                    sb_spawns.Append("'" + aString + "',"); //write our hexadecimal as an ASCII string
                                    npc_name = aString;
                                    item_name = aString;

                                    while ((j % 2 != 0) || (j % 4 == 0))    //padding for strings
                                    {
                                        fs.Read(pos, readIndex, 1);
                                        readIndex++;
                                        j++;
                                    }
                                }
                            }
                            #endregion
                            #region Type
                            /*             
                            //PackedDWORD type (object type)
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);

                            //PackedDWORD icon (icon ResourceID (minus 0x06000000))
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            int wIcon = BitConverter.ToInt32(res, writeIndex - 4);
                            */

                            //WORD type
                            fs.Read(pos, readIndex, 2);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            string poop1 = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            string poop2 = BitConverter.ToString(res, writeIndex - 1, 1);

                            sb.Append(BitConverter.ToInt16(res, writeIndex - 2) + ",");

                            //wModel = poop2 + poop1;
                            dwModelNumber = BitConverter.ToInt16(res, writeIndex - 2);
                            #endregion
                            #region Icon
                            //WORD icon
                            if (npc_name == "Gold Golem")
                            {
                                wIcon = 4644;
                            }
                            else if (npc_name == "Diamond Golem")
                            {
                                wIcon = 4644;
                            }
                            else if (npc_name == "Pyreal Golem")
                            {
                                wIcon = 4644;
                            }
                            else if (npc_name == "Bloodthirsty Monouga")
                            {
                                wIcon = 5821;
                            }
                            else
                            {
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                sb.Append(BitConverter.ToInt16(res, writeIndex - 2) + ",");
                                wIcon = (ushort)BitConverter.ToInt16(res, writeIndex - 2);
                            }
                            #endregion
                            #region Cateogry
                            //ObjectCategoryFlags category (object categories)
                            /* ObjectCategoryFlags (DWORD)
                             * Part one of an object's flags */
                            string[] item_oflags1 = new string[4];
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_oflags1[3] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_oflags1[2] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_oflags1[1] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_oflags1[0] = BitConverter.ToString(res, writeIndex - 1, 1);

                            int category = BitConverter.ToInt32(res, writeIndex - 4);

                            string s_dwObjectFlasg1 = item_oflags1[0] + item_oflags1[1] + item_oflags1[2] + item_oflags1[3];
                            #endregion
                            #region Behavior
                            //ObjectBehaviorFlags behavior (object behaviors)
                            /* ObjectBehaviorFlags (DWORD)
                             * Flags related to the use of the item. */
                            string[] item_oflags2 = new string[4];
                            fs.Read(pos, readIndex, 4);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_oflags2[3] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_oflags2[2] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_oflags2[1] = BitConverter.ToString(res, writeIndex - 1, 1);
                            res.SetValue(pos[readIndex++], writeIndex++);
                            item_oflags2[0] = BitConverter.ToString(res, writeIndex - 1, 1);

                            string s_dwObjectFlasg2 = item_oflags2[0] + item_oflags2[1] + item_oflags2[2] + item_oflags2[3];

                            int behavior = BitConverter.ToInt32(res, writeIndex - 4);
                            #endregion
                            #region Flags2
                            if ((behavior & 0x04000000) == 0x04000000)
                            {
                                //DWORD flags2 (additional game data flag)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion

                            #region FLAGS1
                            #region Name - Plural
                            /*
                            //Flags1 & 0x00000001
                            if ((gameDataFlags1 & 0x00000001) == 0x00000001)
                            {
                                //String namePlural (plural object name (if not specified, use <name> followed by 's' or 'es'))
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                ushort pluralCount = (ushort)BitConverter.ToInt16(res, writeIndex - 2);

                                fs.Read(pos, readIndex, pluralCount);
                                for (int j = 0; j < pluralCount; j++)   //vector of length pluralCount
                                {
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                    if (j == pluralCount)
                                    {
                                        System.Text.Encoding ascii = System.Text.Encoding.ASCII;    //define ASCII format
                                        sb.Append("'" + ascii.GetString(res, writeIndex - pluralCount - 1, pluralCount) + "'"); //write our hexadecimal as an ASCII string

                                        while ((j % 2 != 0) || (j % 4 == 0))    //padding for strings
                                        {
                                            fs.Read(pos, readIndex, 1);
                                            readIndex++;
                                            j++;
                                        }
                                    }
                                }
                            }
                            */
                            #endregion
                            #region Item slots
                            //Flags1 & 0x00000002
                            if ((gameDataFlags1 & 0x00000002) == 0x00000002)
                            {
                                //BYTE itemSlots (number of item slots)
                                fs.Read(pos, readIndex, 1);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                //sb.Append(BitConverter.ToChar(res, index - 1) + ",");
                            }
                            #endregion
                            #region Pack slots
                            //Flags1 & 0x00000004
                            if ((gameDataFlags1 & 0x00000004) == 0x00000004)
                            {
                                //BYTE packSlots (number of pack slots (a pack slot is a slot that may hold a pack or a foci))
                                fs.Read(pos, readIndex, 1);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                //sb.Append(BitConverter.ToChar(res, index - 1) + ",");
                            }
                            #endregion
                            #region Ammo Type
                            //Flags1 & 0x00000100
                            if ((gameDataFlags1 & 0x00000100) == 0x00000100)
                            {
                                //AmmoType ammunition (missile ammunition type)
                                /* AmmoType (WORD)
                                 * The AmmoType value describes the type of ammunition a missile weapon uses. */
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region Item Value
                            //Flags1 & 0x00000008
                            if ((gameDataFlags1 & 0x00000008) == 0x00000008)
                            {
                                //DWORD value (object value)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                ovalue = BitConverter.ToInt16(res, writeIndex - 4);
                            }
                            #endregion
                            #region Unk10
                            //Flags1 & 0x00000010
                            if ((gameDataFlags1 & 0x00000010) == 0x00000010)
                            {
                                //DWORD unknown10
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                int unk10 = BitConverter.ToInt32(res, writeIndex - 4);
                            }
                            #endregion
                            #region Approach Distance
                            //Flags1 & 0x00000020
                            if ((gameDataFlags1 & 0x00000020) == 0x00000020)
                            {
                                //float approachDistance (distance a player will walk to pick up the object)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                //sb.Append(BitConverter.ToSingle(res, writeIndex - 4) + ",");
                            }
                            #endregion
                            #region Usable On
                            //Flags1 & 0x00080000
                            if ((gameDataFlags1 & 0x00080000) == 0x00080000)
                            {
                                //DWORD usableOn (the object categories this object may be used on)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                useableon = BitConverter.ToInt32(res, writeIndex - 4);
                            }
                            #endregion
                            #region Icon Highlight
                            //Flags1 & 0x00000080
                            if ((gameDataFlags1 & 0x00000080) == 0x00000080)
                            {
                                //IconHighlight iconHighlight (the type of highlight (outline) applied to the object's icon)
                                /* IconHighlight (DWORD)
                                 * The IconHighlight value describes the type of highlight (outline) applied to an icon. */
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                iconhigh = BitConverter.ToInt32(res, writeIndex - 4);
                            }
                            #endregion
                            #region Wield Type
                            //Flags1 & 0x00000200
                            if ((gameDataFlags1 & 0x00000200) == 0x00000200)
                            {
                                //WieldType wieldType (the type of wieldable item this is)
                                /* WieldType (BYTE)
                                 * The WieldType value describes a wieldable item's type. */
                                fs.Read(pos, readIndex, 1);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                wieldtype = BitConverter.ToInt32(res, writeIndex - 1);
                            }
                            #endregion
                            #region Uses
                            //Flags1 & 0x00000400
                            if ((gameDataFlags1 & 0x00000400) == 0x00000400)
                            {
                                //WORD uses (the number of uses remaining for this item (also salvage quantity))
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                uses = BitConverter.ToInt32(res, writeIndex - 2);
                            }
                            #endregion
                            #region Use Limit
                            //Flags1 & 0x00000800
                            if ((gameDataFlags1 & 0x00000800) == 0x00000800)
                            {
                                //WORD usesLimit (the maximum number of uses possible for this item (also maximum salvage quantity))
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                uselimit = BitConverter.ToInt32(res, writeIndex - 2);
                            }
                            #endregion
                            #region Stack
                            //Flags1 & 0x00001000
                            if ((gameDataFlags1 & 0x00001000) == 0x00001000)
                            {
                                //WORD stack (the number of items in this stack of objects)
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                stack = BitConverter.ToInt32(res, writeIndex - 2);
                            }
                            #endregion
                            #region Stack Limit
                            //Flags1 & 0x00002000
                            if ((gameDataFlags1 & 0x00002000) == 0x00002000)
                            {
                                //WORD stackLimit (the maximum number of items possible in this stack of objects)
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                stacklimit = BitConverter.ToInt32(res, writeIndex - 2);
                            }
                            #endregion
                            #region Container ID
                            //Flags1 & 0x00004000
                            if ((behavior & 0x00004000) == 0x00004000)
                            {
                                //ObjectID container (the ID of the container holding this object)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region Equipper
                            //Flags1 & 0x00008000
                            if ((gameDataFlags1 & 0x00008000) == 0x00008000)
                            {
                                //ObjectIDequipper (the ID of the creature equipping this object)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region Equip Possible
                            //Flags1 & 0x00010000
                            if ((gameDataFlags1 & 0x00010000) == 0x00010000)
                            {
                                //EquipMask equipPossible (the potential equipment slots this object may be placed in)
                                /* EquipMask (DWORD)
                                 * The EquipMask value describes the equipment slots an item uses. */
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                equippos = BitConverter.ToInt32(res, writeIndex - 4);
                            }
                            #endregion
                            #region Equip Actual
                            //Flags1 & 0x00020000
                            if ((gameDataFlags1 & 0x00020000) == 0x00020000)
                            {
                                //EquipMask equipActual (the actual equipment slots this object is currently placed in)
                                /* EquipMask (DWORD)
                                 * The EquipMask value describes the equipment slots an item uses. */
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                equipact = BitConverter.ToInt32(res, writeIndex - 4);
                            }
                            #endregion
                            #region Coverage
                            //Flags1 & 0x00040000
                            if ((gameDataFlags1 & 0x00040000) == 0x00040000)
                            {
                                //CoverageMask coverage (the parts of the body this object protects)
                                /* CoverageMask (DWORD)
                                 * The CoverageMask value describes what parts of the body an item protects. */
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                coverage = BitConverter.ToInt32(res, writeIndex - 4);
                            }
                            #endregion
                            #region Unk100000
                            //Flags1 & 0x00100000
                            if ((gameDataFlags1 & 0x00100000) == 0x00100000)
                            {
                                fs.Read(pos, readIndex, 1);
                                //BYTE unknown100000
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region unk800000
                            //Flags1 & 0x00800000
                            if ((gameDataFlags1 & 0x00800000) == 0x00800000)
                            {
                                //BYTE unknown800000
                                fs.Read(pos, readIndex, 1);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region unk8000000
                            //Flags1 & 0x08000000
                            if ((gameDataFlags1 & 0x08000000) == 0x08000000)
                            {
                                //WORD unknown8000000
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                unk8000000 = BitConverter.ToInt32(res, writeIndex - 2);
                            }
                            #endregion
                            #region Workmanship
                            //Flags1 & 0x01000000
                            if ((gameDataFlags1 & 0x01000000) == 0x01000000)
                            {
                                //float workmanship (object workmanship)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region Burden
                            //Flags1 & 0x00200000
                            if ((gameDataFlags1 & 0x00200000) == 0x00200000)
                            {
                                //WORD burden (total burden of this object)
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                oburden = BitConverter.ToInt32(res, writeIndex - 1);
                            }
                            #endregion
                            #region Spell ID
                            //Flags1 & 0x00400000
                            if ((gameDataFlags1 & 0x00400000) == 0x00400000)
                            {
                                //SpellID spell (the spell cast by this object)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region Owner
                            //Flags1 & 0x02000000
                            if ((gameDataFlags1 & 0x02000000) == 0x02000000)
                            {
                                //ObjectID owner (the owner of this object)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                //sb.Append(BitConverter.ToInt32(res, writeIndex - 4) + ",");
                            }
                            #endregion
                            #region Dwelling ACL
                            //Flags1 & 0x04000000
                            if ((gameDataFlags1 & 0x04000000) == 0x04000000)
                            {
                                //DwellingACL acl (the access control list for this dwelling object)

                                /* DwellingACL
                                 * The DwellingACL contains the access control list for a dwelling object. */

                                //DWORD flags (believed to be flags that control the size and content of this structure)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                int dwellingFlags = BitConverter.ToInt32(res, writeIndex - 2);

                                //DWORD open (0 = private dwelling, 1 = open to public)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                sb.Append(BitConverter.ToInt32(res, writeIndex - 4) + ",");

                                //ObjectID allegiance (allegiance monarch (if allegiance access granted))
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                //WORD guestCount (number of guests on list)
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                ushort guestCount = (ushort)BitConverter.ToInt16(res, writeIndex - 2);

                                //WORD guestLimit (maximum number of guests on guest list (cottage is 32))
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                if (guestCount <= 0x300)
                                {
                                    for (int j = 0; j < guestCount; j++)    //vector of length guestCount
                                    {
                                        //ObjectID guest (the ID of the guest)
                                        fs.Read(pos, readIndex, 4);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                        res.SetValue(pos[readIndex++], writeIndex++);

                                        //Boolean storage (0 = dwelling access only, 1 = storage access also)
                                        fs.Read(pos, readIndex, 4);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                        res.SetValue(pos[readIndex++], writeIndex++);
                                    }
                                }
                            }
                            #endregion
                            #region Hook Type
                            //Flags1 & 0x20000000
                            if ((gameDataFlags1 & 0x20000000) == 0x20000000)
                            {
                                //WORD hookTypeUnknown (always -1)
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                hooks = BitConverter.ToInt32(res, writeIndex - 4);
                            }
                            #endregion
                            #region ObjectIDs Monarch
                            //Flags1 & 0x00000040
                            if ((gameDataFlags1 & 0x00000040) == 0x00000040)
                            {
                                //ObjectID monarch (this player's monarch)
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region Hookable On
                            //Flags1 & 0x10000000
                            if ((gameDataFlags1 & 0x10000000) == 0x10000000)
                            {
                                //HookType hookableOn (the types of hooks this object may be placed on (-1 for hooks))
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #region Icon Overlay
                            //Flags1 & 0x40000000
                            if ((gameDataFlags1 & 0x40000000) == 0x40000000)
                            {
                                //PackedDWORD iconOverlay (icon overlay ResourceID (minus 0x06000000))
                                fs.Read(pos, readIndex, 2);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);

                                //Behavior & 0x40000000
                                if ((behavior & 0x04000000) == 0x04000000)
                                {
                                    //PackedDWORD iconUnderlay (icon underlay ResourceID (minus 0x06000000))
                                    fs.Read(pos, readIndex, 4);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                    res.SetValue(pos[readIndex++], writeIndex++);
                                }
                            }
                            #endregion
                            #region Material Type
                            //Flags1 & 0x80000000
                            if ((gameDataFlags1 & 0x80000000) == 0x80000000)
                            {
                                //MaterialType material (the type of material this object is made of)
                                /* MaterialType (DWORD)
                                 * The MaterialType identifies the material an object is made of. */
                                fs.Read(pos, readIndex, 4);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                                res.SetValue(pos[readIndex++], writeIndex++);
                            }
                            #endregion
                            #endregion
                        #endregion


                            /** Determine how to write the SQL statement **/
                            //Insert object into the database based upon a unique characteristic.
                            #region SQL Writting
                            //if (chkItems.Checked)
                            //{
                            switch (category)
                            {
                                case 0x00000001:
                                    {
                                        if (chkWeapons.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iWeapons++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00000002:
                                    {
                                        if (chkArmor.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + "," + unk10 + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "'," + ovalue + "," + equippos + "," + coverage + "," + oburden + "," + hooks + ");");
                                                sw.WriteLine();
                                                iArmor++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00000004:
                                    {
                                        if (chkClothes.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iClothes++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00000008:
                                    {
                                        if (chkJewelry.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iJewelry++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }

                                case 0x00000010:
                                    {
                                        if (chkMonPTM.Checked)
                                        {
                                            if (behavior == 0x0014)
                                            {
                                                //Check whether the monster has already been processed during the current execution
                                                sameName = false;
                                                for (int j = 0; j < monster_name_array.Count; j++)
                                                {
                                                    if (monster_name_array[j].Equals(npc_name))
                                                        sameName = true;
                                                }
                                                if (sameName == false)
                                                {
                                                    monster_name_array.Add(npc_name);

                                                    //Cube:  Need to see if the monster already is in the database
                                                    MySqlCommand findmonster = new MySqlCommand("SELECT * FROM monsters_templates WHERE name = ?monname", dbConn);
                                                    findmonster.Parameters.AddWithValue("?monname", npc_name);
                                                    MySqlDataReader reader = findmonster.ExecuteReader();
                                                    while (!reader.Read())
                                                    {
                                                        string unkBytes = "00003D0000000000";
                                                        Monster_PTM(paletteCount, textureCount, modelCount, sw);
                                                        if (!((physicsFlags & 0x00000080) == 0x00000080))
                                                            sw.Write(sb_monsters_templates + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "'," + wPortalMode + "," + unknown1 + "," + dwModelNumber + ",NULL," + unknown100 + "," + wIcon + "," + m_Anim + "," + m_Sound + "," + m_res + ",'" + wModel + "'," + unk10 + "," + unk8000000 + "," + byteCount + ",'" + unknownBytes + "'," + c_item_palette_mv + "," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                        else
                                                            sw.Write(sb_monsters_templates + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "'," + wPortalMode + "," + unknown1 + "," + dwModelNumber + "," + scale + "," + unknown100 + "," + wIcon + "," + m_Anim + "," + m_Sound + "," + m_res + ",'" + wModel + "'," + unk10 + "," + unk8000000 + "," + byteCount + ",'" + unknownBytes + "'," + c_item_palette_mv + "," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                        sw.WriteLine();
                                                        iMonsterPTM++;
                                                        break;
                                                    }
                                                    reader.Close();
                                                }
                                            }
                                        }

                                        if (chkMonSpawns.Checked)
                                        {
                                            if (behavior == 0x0014)
                                            {
                                                //MySqlCommand findmodelnum = new MySqlCommand("SELECT * FROM monsters_templates WHERE wicon = ?micon", dbConn);
                                                //findmodelnum.Parameters.AddWithValue("?micon", wIcon);
                                                //MySqlDataReader reader = findmodelnum.ExecuteReader();
                                                //int dwLinker = reader.GetOrdinal("dwLinker");
                                                //while (reader.Read())
                                                //{
                                                //sb_spawns.Append(reader.GetUInt32(dwLinker));
                                                sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                                sw.WriteLine(sb_spawns + ");");
                                                //}
                                                //reader.Close();
                                                iMonsterSpawns++;
                                                break;
                                            }
                                            break;
                                        }
                                        else
                                            break;
                                    }

                                case 0x00000020:
                                    {
                                        if (chkFood.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "'," + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iFood++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                /*
                                case 0x00000040:
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                 */
                                case 0x00000080:
                                    {
                                        if (chkDoors.Checked)
                                        {
                                            if (behavior == 0x1014)
                                            {
                                                iDoors++;
                                                sw.WriteLine("INSERT INTO `doors` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `AnimConfig`, `SoundSet`, `Type`, `Name`, `wModel`, `wIcon`) VALUES");
                                                sb.Remove(sb.Length - 1, 1);
                                                sw.WriteLine(sb + ");");
                                                sw.WriteLine();
                                                iDoors++;
                                                break;
                                            }
                                            break;
                                        }



                                        if (behavior == 0x0014)
                                        {
                                            if (chkMerchantSigns.Checked)
                                            {
                                                MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                                finditem.Parameters.AddWithValue("?itemname", item_name);
                                                MySqlDataReader reader = finditem.ExecuteReader();
                                                while (!reader.Read())
                                                {
                                                    iMerchantSigns++;
                                                    Item_PTM(paletteCount, "worldobjects_vector_palettes", textureCount, "worldobjects_vector_textures", modelCount, "worldobjects_vector_models", sw);
                                                    sw.Write(sb_worldobj2modeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                    sw.WriteLine();
                                                    sw.Write(sb_worldobjects2 + "'" + item_name + "'," + c_item_palette_mv + "," + wo2lb + "," + wo2x + "," + wo2y + "," + wo2z + "," + wo2wQuat + "," + wo2xQuat + "," + wo2yQuat + "," + wo2zQuat + "," + dwModelNumber + "," + wIcon + ");");
                                                    sw.WriteLine();
                                                    //TODO: Complete this section.
                                                    iMerchantSigns++;
                                                    break;
                                                }
                                                reader.Close();
                                                break;
                                            }
                                            else
                                                break;
                                        }
                                        if (chkMisc.Checked)
                                        {
                                            if (behavior == 0x0012)
                                            {
                                                MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                                finditem.Parameters.AddWithValue("?itemname", item_name);
                                                MySqlDataReader reader = finditem.ExecuteReader();
                                                while (!reader.Read())
                                                {
                                                    Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                    sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                    sw.WriteLine();
                                                    iMisc++;
                                                    break;
                                                }
                                                reader.Close();
                                                break;
                                            }
                                            else
                                                break;
                                        }
                                        break;
                                    }
                                case 0x00000100:
                                    {
                                        if (chkAmmo.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iAmmo++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00000200:
                                    {
                                        if (chkPacks.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iPack++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00000400:
                                    {
                                        if (chkDecor.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iDecor++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00000800:
                                    {
                                        if (chkGems.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iGems++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00001000:
                                    {
                                        if (chkComps.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "'," + ovalue + "," + 4 + ");");
                                                sw.WriteLine();
                                                iComps++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00002000:
                                    {
                                        if (chkBooks.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iBooks++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00004000:
                                    {
                                        if (chkKeys.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iKeys++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00008000:
                                    {
                                        if (chkWand.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "'," + ovalue + "," + oburden + ");");
                                                sw.WriteLine();
                                                iWands++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }

                                case 0x00010000:
                                    {
                                        if (chkPortals.Checked)
                                        {
                                            sw.WriteLine("INSERT INTO `gameobjects_portals` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `AnimConfig`, `Type`, `Name`, `wModel`, `wIcon`) VALUES");
                                            //sw.WriteLine("INSERT INTO `gameobjects_portals` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Name`, `color`, `DestLB`, `DestX`, `DestY`, `DestZ`, `DestH`, `DestU1`, `DestU2`, `DestH2`, `min_lvl`, `max_lvl`) VALUES");
                                            sb.Remove(sb.Length - 1, 1);
                                            sw.WriteLine(sb + ")");
                                            sw.WriteLine();
                                            iPortals++;
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                /*
                                case 0x00020000:
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                */
                                case 0x00040000:
                                    {
                                        if (chkNotes.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iNotes++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }

                                case 0x00080000:
                                    {
                                        if (chkManastones.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "'," + ovalue + "," + oburden + ");");
                                                sw.WriteLine();
                                                iManastones++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00400000:
                                    {
                                        if (chkCook.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iCook++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x00800000:
                                    {
                                        if (chkFletch.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "'," + ovalue + "," + oburden + ");");
                                                sw.WriteLine();
                                                iFletch++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x04000000:
                                    {
                                        if (chkAlchemy.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iAlchemy++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x20000000:
                                    {
                                        if (chkMisc.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iMisc++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                                case 0x40000000:
                                    {
                                        if (chkSalvage.Checked)
                                        {
                                            MySqlCommand finditem = new MySqlCommand("SELECT * FROM items_templates WHERE name = ?itemname", dbConn);
                                            finditem.Parameters.AddWithValue("?itemname", item_name);
                                            MySqlDataReader reader = finditem.ExecuteReader();
                                            while (!reader.Read())
                                            {
                                                Item_PTM(paletteCount, "items_vector_palettes", textureCount, "items_vector_textures", modelCount, "items_vector_models", sw);
                                                sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                                sw.WriteLine();
                                                iSalvage++;
                                                break;
                                            }
                                            reader.Close();
                                            break;
                                        }
                                        else
                                            break;
                                    }
                            }
                            //}

                            switch (wIcon)
                            {
                                case 0x218C:    //crystal covenant
                                    if (chkCovenantCrystals.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `covenants_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `AnimConfig`, `Type`, `Name`, `wModel`, `wIcon`, `Description`) VALUES");
                                        sb.Append("'Covenant Crystal',");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ");");
                                        sw.WriteLine();
                                        iCovenants++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x2181:    //house (apartment / cottage)
                                    if (chkApartments.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `houses_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Type`, `Name`, `wModel`, `wIcon`, `isOpen`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ")");
                                        sw.WriteLine();
                                        iApartments++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x218E:    //house (villa)
                                    if (chkVillas.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `houses_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Type`, `Name`, `wModel`, `wIcon`, `isOpen`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ")");
                                        sw.WriteLine();
                                        iVillas++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0xC129:    //house (mansion)
                                    if (chkMansions.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `houses_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Type`, `Name`, `wModel`, `wIcon`, `isOpen`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ")");
                                        sw.WriteLine();
                                        iMansions++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x20C0:    //housing hook
                                    if (chkHooks.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `hooks_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Type`, `Name`, `wModel`, `wIcon`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ")");
                                        sw.WriteLine();
                                        iHooks++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x218D:    //housing storage
                                    if (chkHouseChests.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `storage_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `AnimConfig`, `SoundSet`, `Type`, `Name`, `wModel`, `wIcon`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ")");
                                        sw.WriteLine();
                                        iHouseChests++;
                                        break;
                                    }
                                    else
                                        break;
                                /*
                        case 0x1317:    //door
                            if (chkDoors.Checked)
                            {
                                sw.WriteLine("INSERT INTO `doors` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `AnimConfig`, `SoundSet`, `Type`, `Name`, `wModel`, `wIcon`) VALUES");
                                sb.Remove(sb.Length - 1, 1);
                                sw.WriteLine(sb + ");");
                                sw.WriteLine();
                                iDoors++;
                                break;
                            }
                            else
                                break;
                                 */
                                case 0x1020:    //general chest
                                    if (chkChests.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `chests_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `AnimConfig`, `SoundSet`, `Type`, `Name`, `wModel`, `wIcon`,) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ")");
                                        sw.WriteLine();
                                        iChests++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x1FE9:    //singularity trove
                                    if (chkTroves.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `chests_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `AnimConfig`, `SoundSet`, `Type`, `Name`, `wModel`, `wIcon`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ")");
                                        sw.WriteLine();
                                        iTroves++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x1355:    //lifestone
                                    if (chkLifestones.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `lifestone_test` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `AnimConfig`, `SoundSet`, `Type`, `Name`, `wModel`, `wIcon`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ")");
                                        sw.WriteLine();
                                        iLifestones++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x1036:    //npcs
                                    if (chkNPCs.Checked)
                                    {
                                        //Advance the counts
                                        //c_npc_model_id++;
                                        //c_npc_palette_id++;
                                        //c_npc_texture_id++;

                                        //sb.Remove(sb_npcs.Length -1, 1);

                                        //Write initial SQL value
                                        sw.WriteLine(initial_line);

                                        //Add Palette info
                                        for (int j = 0; j < paletteCount; j++)
                                        {
                                            //Cube:  Have to assign replace to a new string :p
                                            fix_pal = sNewPalette[j].Replace("-", "");

                                            sw.Write(fix_pal);
                                            sw.Write(sPaletteOffset[j]);
                                            sw.Write(sPaletteLength[j]);
                                            sw.WriteLine();
                                        }

                                        sw.WriteLine(insert_tex);

                                        //Add the texture info
                                        for (int j = 0; j < textureCount; j++)
                                        {

                                            sw.Write(sTModelIndex[j]);
                                            sw.Write(sNewTexture2[j]);
                                            sw.Write(sNewTexture1[j]);
                                            sw.Write(sOldTexture2[j]);
                                            sw.Write(sOldTexture1[j]);

                                            sw.WriteLine();
                                        }

                                        sw.WriteLine(insert_mod);

                                        //Add model info
                                        for (int j = 0; j < modelCount; j++)
                                        {
                                            sw.Write(sMModelIndex[j]);
                                            sw.Write(sNewModel2[j]);
                                            sw.Write(sNewModel1[j]);
                                            sw.WriteLine();
                                        }

                                        sb_npcs.Append(c_npc_palette_mv.ToString() + "," + "'" + npc_name + "'," + "'m'," + c_npc_palette_mv);
                                        sw.WriteLine(sb_npcs + ");");
                                        //(Modelnum, ModelName, PaletteVector, ucPaletteChangeCount, TextureVector, ucTextureChangeCount, ModelVector, ucModelVectorCount, wModelID, wIconID, AnimationStrip, unkDataEntry, unkDataEntry2, wPaletteCode)
                                        sb_modeldata.Append(c_npc_palette_mv + ",'" + npc_name + "'," + c_npc_palette_mv + "," + paletteCount + "," + c_npc_palette_mv + "," + textureCount + "," + c_npc_palette_mv + "," + modelCount + ",'" + wModel + "', '1036', '0001', '0001', '0001', '0001','" + wPaletteCode + "');");
                                        sw.WriteLine(sb_modeldata);
                                        iNPCs++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x12D3:
                                    if (chkTownSigns.Checked)
                                    {
                                        //Signs
                                        sw.WriteLine("INSERT INTO `worldobjects` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Type`, `Name`, `wModel`, `wIcon`, `iObjecttype`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ",0);");
                                        sw.WriteLine();
                                        iTownSigns++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x2356:    //Agent of Arcanum Signs
                                    if (chkTownSigns.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `worldobjects` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Type`, `Name`, `wModel`, `wIcon`, `iObjecttype`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ",0);");
                                        sw.WriteLine();
                                        iTownSigns++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x104A:    //Wells
                                    if (chkWells.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `worldobjects` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Type`, `Name`, `wModel`, `wIcon`, `iObjecttype`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ",2);");
                                        sw.WriteLine();
                                        iWells++;
                                        break;
                                    }
                                    else
                                        break;

                                case 0x12C8:       //Fountain
                                    if (chkFountains.Checked)
                                    {
                                        sw.WriteLine("INSERT INTO `worldobjects` (`Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`, `Type`, `Name`, `wModel`, `wIcon`, `iObjecttype`) VALUES");
                                        sb.Remove(sb.Length - 1, 1);
                                        sw.WriteLine(sb + ",2);");
                                        sw.WriteLine();
                                        iFountains++;
                                        break;
                                    }
                                    else
                                        break;
                                //Items Commented out.
                                #region Items
                                /*
                                //case 0x229F:        //Pyreals
                                    if (chkItems.Checked)
                                    {
                                        //Cube: This needs to be a little different than NPCs, sometimes there is no
                                        //p/t/m info in one column but are in others.
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        //sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + wModel + "," + wIcon + ",'" + dwModelNumber + "'," + c_item_palette_mv);
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1BE3:        //Amuli coat
                                    if (chkItems.Checked)
                                    {
                                        //Cube: This needs to be a little different than NPCs, sometimes there is no
                                        //p/t/m info in one column but are in others.
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + wModel + "," + wIcon + ",'" + dwModelNumber + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "'," + ovalue + "," + iconhigh + "," + wieldtype + "," + uses + "," + uselimit + "," + stack + "," + stacklimit + "," + equippos + "," + equipact + "," + coverage + "," + oburden);
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1BEA:        //Amuli Legs
                                    if (chkItems.Checked)
                                    {
                                        //Cube: This needs to be a little different than NPCs, sometimes there is no
                                        //p/t/m info in one column but are in others.
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        //sw.WriteLine(sb_itemmodeldata);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + "," + physicsFlags + ",");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                //case 0x1BB7:        //Pack
                                    if (chkItems.Checked)
                                    {
                                        //Cube: This needs to be a little different than NPCs, sometimes there is no
                                        //p/t/m info in one column but are in others.
                                        Item_PTM(paletteCount,textureCount,modelCount,sw);
                                        sw.WriteLine(sb_itemmodeldata);
                                        break;
                                    }
                                    else
                                        break;
                                //case 0x15CC:        //Dagger
                                    if (chkItems.Checked)
                                    {
                                        //Cube: This needs to be a little different than NPCs, sometimes there is no
                                        //p/t/m info in one column but are in others.
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.WriteLine(sb_itemmodeldata);
                                        break;
                                    }
                                    else
                                        break;
                                case 0x16C9:        //C-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x16CA:        //D-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x16CB:        //I-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x16CC:        //L-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x16CD:        //V-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x16CE:        //X-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x16CF:        //M-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1942:        //Throwing Club
                                    if (chkItems.Checked)
                                    {
                                        break;
                                    }
                                    else
                                        break;
                                case 0x275F:        //MD-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2760:        //MM-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2761:        //MMD-Note
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_palette_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');"); 
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;

                                    //Armor
                                case 0x1311:        //Studded Leather Boots
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FC4:        //Studded Leather Bracers
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x121A:        //Studded Leather Breastplate
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FBB:        //Studded Leather Cowl
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FCE:        //Studded Leather Gauntlets
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1306:        //Metal Cap
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x12F1:        //Studded Leather Girth
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x12DB:        //Studded Leather Greaves
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x5DE2:        //Studded Leather Tassets
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x121B:        //Studded Leather Leggings
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x18FF:        //Studded Leather Pauldrons
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x142D:        //Round Shield
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FCF:        //Chainmail Basinet
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1315:        //Chainmail Bracers
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1B37:        //Chainmail Breastplate
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FCB:        //Chainmail Gauntlets
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x12EE:        //Chainmail Girth
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1584:        //Chainmail Greaves
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2730:        //Chainmail Tassets
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FC6:        //Chainmail Leggings
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x130A:        //Chainmail Pauldrons
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FCA:        //Chainmail Coif
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1F5B:        //Yoroi Cuirass
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x12F3:        //Yoroi Breastplate
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x12F2:        //Yoroi Girth
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FDC:        //Yoroi Leggings
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x130F:        //Yoroi Pauldrons
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13FB:        //Yoroi Sleeves
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FAD:        //Sollerets
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1228:        //Kabuton
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x0FC3:        //Kote
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1426:        //Buckler
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1436:        //Large Round Shield
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x14B0:        //Tower Shield
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1D6C:        //Plain Lockpick
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1D6D:        //Reliable Lockpick
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1D6A:        //Good Lockpick
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32EF:        //Handy Healing Kit
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13E7:        //Lead Scarab
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13E6:        //Iron Scarab
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13E4:        //Copper Scarab
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13E9:        //Silver Scarab
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13E5:        //Gold Scarab
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13E8:        //Pyreal Scarab
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x262A:        //Prismatic Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1401:        //Amaranth
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1007:        //Bistort
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1402:        //Comfrey
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1403:        //Damiana
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1404:        //DragonsBlood
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1405:        //Eyebright
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1406:        //Frankisence
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1407:        //Ginseng
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1408:        //Hawthorn
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1409:        //Henbane
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x140A:        //Hyssop
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x140B:        //Mandrake
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x140C:        //Mugwort
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x140D:        //Myrrh
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x140F:        //Saffron
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x140E:        //Vervain
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1410:        //Wormwood
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1411:        //Yarrow
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13D9:        //Powdered Agate
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13DF:        //Powdered Amber
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1907:        //Powdered Azurite
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13DB:        //Powdered Bloodstone
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13E1:        //Powdered Carnelian 
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13DD:        //Powdered Hematite 
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1906:        //Powdered Lapus Lazuli
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1908:        //Powdered Malachite 
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13E2:        //Powdered Moonstone
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13D8:        //Powdered Onyx
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1909:        //Powdered Quartz
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13DA:        //Powdered Turqiose
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13D7:        //Brimstone
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13D6:        //Cadmia
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13D5:        //Cinnabar
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13D0:        //Cobalt
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1905:        //Colcothar
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1902:        //Gypsum
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13D2:        //Quicksilver
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1911:        //Realgar
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1904:        //Stibnite
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1903:        //Turpeth
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13D1:        //Verdigis
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1912:        //Vitriol
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x190C:        //Poplar Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13EA:        //Blackthorn Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x190B:        //Yew Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x190E:        //Hemlock Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13EC:        //Alder Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x190D:        //Ebony Talisman 
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1910:        //Birch Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13EF:        //Ashwood Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13ED:        //Elder Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x190A:        //Rowan Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13F0:        //Willow Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x190F:        //Cedar Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13EE:        //Oak Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13EB:        //Hazel Talisman
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A6:        //Red Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A5:        //Pink Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A4:        //Orange Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A9:        //Yellow Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A1:        //Green Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x139F:        //Turqoise Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x139E:        //Blue Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A3:        //Indigo Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A7:        //Violet Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A0:        //Brown Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A8:        //White Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x13A2:        //Gray Taper
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32CE:        //Minor Mana Stone
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32CF:        //Lesser Mana Stone
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32D0:        //Mana Stone
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32D2:        //Tiny Mana Charge
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32C9:        //Small Mana Charge
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32CA:        //Moderate Mana Charge
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32CB:        //High Mana Charge
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x32CC:        //Great Mana Charge
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1BA8:        //Suikan Creature/Item/Life/War Apprentice Robe
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1A27:        //Alembic
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1AA4:        //Mortar and Pestle
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1A28:        //Aqua Incanta
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1A34:        //Neutral Balm 
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x157E:        //Wand
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1532:        //Orb
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x16A7:        //Staff
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1AFB:        //Arrowheads
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1A3F:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1AD5:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1B1B:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1AF4:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1B1E:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1AF2:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x1B1A:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x29FD:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x29FC:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x207B:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x206B:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x206A:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2069:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x206D:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2067:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2079:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2374:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2375:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2457:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                case 0x2463:        //Arrow Shafts
                                    if (chkItems.Checked)
                                    {
                                        Item_PTM(paletteCount, textureCount, modelCount, sw);
                                        sw.Write(sb_itemmodeldata + "'" + item_name + "'," + paletteCount + "," + textureCount + "," + modelCount + ",'" + s_dwFlags1 + "','" + wPaletteCode + "'," + dwModelNumber + "," + wIcon + ",'" + wModel + "'," + c_item_model_mv + ",'" + s_dwFlags2 + "'," + "'" + s_dwObjectFlasg1 + "'," + "'" + s_dwObjectFlasg2 + "');");
                                        sw.WriteLine();
                                        break;
                                    }
                                    else
                                        break;
                                     
                                    #endregion

                                    #region Monsters
                                    case 0x121F:    //Armedillo
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x210C:    //Aun Tumerok
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1220:    //Auroch
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x103D:    //Banderling
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x210A:    //Carenzi
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1034:    //Cow
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1B4B:    //Crystal
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1FBD:    //Doll
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1035:    //Drudge
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1B42:    //Fire Elemental
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1BBB:    //Dual Fragment
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x2402:    //Frost Elemental
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1224:    //Golem
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1DF0:    //Greivver
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1222:    //Gromnie
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x103C:    //Hea Tumerok
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1EE4:    //Idol
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1C75:    //Lightning Elemental
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1037:    //Lugian
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x16C1:    //Mattekar
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1ED1:    //Moarsman
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x16BD:    //Monouga
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1039:    //Mosswart
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x10E7:    //Olthoi
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x2C42:    //Olthoi (Needler)
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x16BC:    //Rabbit
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x103B:    //Rat
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1BBD:    //Shadow
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1223:    //Shallows Shark/Reedshark
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1918:    //Shreth
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x2107:    //Siraluun
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;

                                    case 0x16C4:    //Skeleton
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;

                                    case 0x1ED2:    //Slithis
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1033:    //Tusker
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1226:    //Undead
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1DEF:    //Ursuin
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x1227:    //Virindi
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x103A:    //Wasp
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x141A:    //Wisp
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;
                                    case 0x16C3:    //Zefir
                                        if (chkSpawns.Checked)
                                        {
                                            imonsterspawns++;
                                            sb_spawns.Remove(sb_spawns.Length - 1, 1);
                                            sw.WriteLine(sb_spawns + ");");
                                            break;
                                        }
                                        else
                                            break;

                                        */
                                    #endregion
                            }
                            #endregion
                        }
                    }
                Skip:
                    i++;

                }
            }

            catch (Exception ex)
            {
                string fserror = ex.Message;
                MessageBox.Show(ex.ToString());
                //MessageBox.Show(fserror,"An Error Occured",MessageBoxButtons.OK,MessageBoxIcon.Error);
            }
        }
    }
}