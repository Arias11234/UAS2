namespace Vendor_Editor
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.databaseSettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.programSettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutVendorEditorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.cmbVendorName = new System.Windows.Forms.ComboBox();
            this.lblVendorName = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.btnAddItem = new System.Windows.Forms.Button();
            this.btnRemoveItem = new System.Windows.Forms.Button();
            this.chkWeapons = new System.Windows.Forms.CheckBox();
            this.chkMissileWeapon = new System.Windows.Forms.CheckBox();
            this.chkFletch = new System.Windows.Forms.CheckBox();
            this.chkArmor = new System.Windows.Forms.CheckBox();
            this.chkSpellComps = new System.Windows.Forms.CheckBox();
            this.chkGems = new System.Windows.Forms.CheckBox();
            this.chkAmmo = new System.Windows.Forms.CheckBox();
            this.lstVendorItems = new System.Windows.Forms.ListBox();
            this.cmbItemType = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.BackColor = System.Drawing.Color.Transparent;
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.aboutToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(572, 24);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.databaseSettingsToolStripMenuItem,
            this.programSettingsToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.ForeColor = System.Drawing.Color.Red;
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // databaseSettingsToolStripMenuItem
            // 
            this.databaseSettingsToolStripMenuItem.BackColor = System.Drawing.Color.Black;
            this.databaseSettingsToolStripMenuItem.ForeColor = System.Drawing.Color.Red;
            this.databaseSettingsToolStripMenuItem.Name = "databaseSettingsToolStripMenuItem";
            this.databaseSettingsToolStripMenuItem.Size = new System.Drawing.Size(173, 22);
            this.databaseSettingsToolStripMenuItem.Text = "Database Settings";
            this.databaseSettingsToolStripMenuItem.Click += new System.EventHandler(this.databaseSettingsToolStripMenuItem_Click);
            // 
            // programSettingsToolStripMenuItem
            // 
            this.programSettingsToolStripMenuItem.BackColor = System.Drawing.Color.Black;
            this.programSettingsToolStripMenuItem.ForeColor = System.Drawing.Color.Red;
            this.programSettingsToolStripMenuItem.Name = "programSettingsToolStripMenuItem";
            this.programSettingsToolStripMenuItem.Size = new System.Drawing.Size(173, 22);
            this.programSettingsToolStripMenuItem.Text = "Program Settings";
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.BackColor = System.Drawing.Color.Black;
            this.exitToolStripMenuItem.ForeColor = System.Drawing.Color.Red;
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(173, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutVendorEditorToolStripMenuItem});
            this.aboutToolStripMenuItem.ForeColor = System.Drawing.Color.Red;
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(48, 20);
            this.aboutToolStripMenuItem.Text = "About";
            // 
            // aboutVendorEditorToolStripMenuItem
            // 
            this.aboutVendorEditorToolStripMenuItem.BackColor = System.Drawing.Color.Black;
            this.aboutVendorEditorToolStripMenuItem.ForeColor = System.Drawing.Color.Red;
            this.aboutVendorEditorToolStripMenuItem.Name = "aboutVendorEditorToolStripMenuItem";
            this.aboutVendorEditorToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.aboutVendorEditorToolStripMenuItem.Text = "About Vendor Editor";
            this.aboutVendorEditorToolStripMenuItem.Click += new System.EventHandler(this.aboutVendorEditorToolStripMenuItem_Click);
            // 
            // cmbVendorName
            // 
            this.cmbVendorName.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbVendorName.ForeColor = System.Drawing.Color.Black;
            this.cmbVendorName.FormattingEnabled = true;
            this.cmbVendorName.Location = new System.Drawing.Point(15, 56);
            this.cmbVendorName.Name = "cmbVendorName";
            this.cmbVendorName.Size = new System.Drawing.Size(233, 21);
            this.cmbVendorName.TabIndex = 1;
            this.cmbVendorName.TextChanged += new System.EventHandler(this.SelectVendor);
            // 
            // lblVendorName
            // 
            this.lblVendorName.AutoSize = true;
            this.lblVendorName.BackColor = System.Drawing.Color.Transparent;
            this.lblVendorName.ForeColor = System.Drawing.Color.White;
            this.lblVendorName.Location = new System.Drawing.Point(25, 40);
            this.lblVendorName.Name = "lblVendorName";
            this.lblVendorName.Size = new System.Drawing.Size(75, 13);
            this.lblVendorName.TabIndex = 2;
            this.lblVendorName.Text = "Vendor Name:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.BackColor = System.Drawing.Color.Transparent;
            this.label1.ForeColor = System.Drawing.Color.White;
            this.label1.Location = new System.Drawing.Point(12, 80);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(88, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Vendor Inventory";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.BackColor = System.Drawing.Color.Transparent;
            this.label2.ForeColor = System.Drawing.Color.White;
            this.label2.Location = new System.Drawing.Point(278, 80);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(50, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "DB Items";
            // 
            // btnAddItem
            // 
            this.btnAddItem.BackColor = System.Drawing.Color.Transparent;
            this.btnAddItem.ForeColor = System.Drawing.Color.White;
            this.btnAddItem.Location = new System.Drawing.Point(373, 301);
            this.btnAddItem.Name = "btnAddItem";
            this.btnAddItem.Size = new System.Drawing.Size(129, 23);
            this.btnAddItem.TabIndex = 5;
            this.btnAddItem.Text = "Add Selected Items";
            this.btnAddItem.UseVisualStyleBackColor = false;
            this.btnAddItem.Click += new System.EventHandler(this.btnAddItem_Click);
            // 
            // btnRemoveItem
            // 
            this.btnRemoveItem.BackColor = System.Drawing.Color.Transparent;
            this.btnRemoveItem.ForeColor = System.Drawing.Color.White;
            this.btnRemoveItem.Location = new System.Drawing.Point(15, 301);
            this.btnRemoveItem.Name = "btnRemoveItem";
            this.btnRemoveItem.Size = new System.Drawing.Size(127, 23);
            this.btnRemoveItem.TabIndex = 6;
            this.btnRemoveItem.Text = "Delete Selected Items";
            this.btnRemoveItem.UseVisualStyleBackColor = false;
            this.btnRemoveItem.Click += new System.EventHandler(this.btnRemoveItem_Click);
            // 
            // chkWeapons
            // 
            this.chkWeapons.Location = new System.Drawing.Point(0, 0);
            this.chkWeapons.Name = "chkWeapons";
            this.chkWeapons.Size = new System.Drawing.Size(104, 24);
            this.chkWeapons.TabIndex = 0;
            // 
            // chkMissileWeapon
            // 
            this.chkMissileWeapon.Location = new System.Drawing.Point(0, 0);
            this.chkMissileWeapon.Name = "chkMissileWeapon";
            this.chkMissileWeapon.Size = new System.Drawing.Size(104, 24);
            this.chkMissileWeapon.TabIndex = 0;
            // 
            // chkFletch
            // 
            this.chkFletch.Location = new System.Drawing.Point(0, 0);
            this.chkFletch.Name = "chkFletch";
            this.chkFletch.Size = new System.Drawing.Size(104, 24);
            this.chkFletch.TabIndex = 0;
            // 
            // chkArmor
            // 
            this.chkArmor.Location = new System.Drawing.Point(0, 0);
            this.chkArmor.Name = "chkArmor";
            this.chkArmor.Size = new System.Drawing.Size(104, 24);
            this.chkArmor.TabIndex = 0;
            // 
            // chkSpellComps
            // 
            this.chkSpellComps.Location = new System.Drawing.Point(0, 0);
            this.chkSpellComps.Name = "chkSpellComps";
            this.chkSpellComps.Size = new System.Drawing.Size(104, 24);
            this.chkSpellComps.TabIndex = 0;
            // 
            // chkGems
            // 
            this.chkGems.Location = new System.Drawing.Point(0, 0);
            this.chkGems.Name = "chkGems";
            this.chkGems.Size = new System.Drawing.Size(104, 24);
            this.chkGems.TabIndex = 0;
            // 
            // chkAmmo
            // 
            this.chkAmmo.Location = new System.Drawing.Point(0, 0);
            this.chkAmmo.Name = "chkAmmo";
            this.chkAmmo.Size = new System.Drawing.Size(104, 24);
            this.chkAmmo.TabIndex = 0;
            // 
            // lstVendorItems
            // 
            this.lstVendorItems.ForeColor = System.Drawing.Color.Black;
            this.lstVendorItems.FormattingEnabled = true;
            this.lstVendorItems.Location = new System.Drawing.Point(15, 96);
            this.lstVendorItems.Name = "lstVendorItems";
            this.lstVendorItems.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.lstVendorItems.Size = new System.Drawing.Size(225, 199);
            this.lstVendorItems.TabIndex = 10;
            // 
            // cmbItemType
            // 
            this.cmbItemType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbItemType.FormattingEnabled = true;
            this.cmbItemType.Location = new System.Drawing.Point(281, 56);
            this.cmbItemType.Name = "cmbItemType";
            this.cmbItemType.Size = new System.Drawing.Size(234, 21);
            this.cmbItemType.TabIndex = 11;
            this.cmbItemType.SelectedIndexChanged += new System.EventHandler(this.ClickHandler);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.BackColor = System.Drawing.Color.Transparent;
            this.label3.ForeColor = System.Drawing.Color.White;
            this.label3.Location = new System.Drawing.Point(278, 40);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(57, 13);
            this.label3.TabIndex = 12;
            this.label3.Text = "Item Type:";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.ClientSize = new System.Drawing.Size(572, 355);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.cmbItemType);
            this.Controls.Add(this.lstVendorItems);
            this.Controls.Add(this.btnRemoveItem);
            this.Controls.Add(this.btnAddItem);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lblVendorName);
            this.Controls.Add(this.cmbVendorName);
            this.Controls.Add(this.menuStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.Text = "Vendor Editor";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem databaseSettingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutVendorEditorToolStripMenuItem;
        private System.Windows.Forms.ComboBox cmbVendorName;
        private System.Windows.Forms.Label lblVendorName;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnAddItem;
        private System.Windows.Forms.Button btnRemoveItem;
        private System.Windows.Forms.CheckBox chkWeapons;
        private System.Windows.Forms.CheckBox chkAmmo;
        private System.Windows.Forms.CheckBox chkGems;
        private System.Windows.Forms.CheckBox chkSpellComps;
        private System.Windows.Forms.CheckBox chkArmor;
        private System.Windows.Forms.ToolStripMenuItem programSettingsToolStripMenuItem;
        private System.Windows.Forms.ListBox lstVendorItems;
        //private System.Windows.Forms.ListBox lstDBItems;
        private System.Windows.Forms.CheckBox chkFletch;
        private System.Windows.Forms.CheckBox chkMissileWeapon;
        private System.Windows.Forms.ComboBox cmbItemType;
        private System.Windows.Forms.Label label3;
    }
}

