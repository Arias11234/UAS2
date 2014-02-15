/*
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
* along with UASv1; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
 *	@file Corpse.cpp
 *	Implements functionality for corpses.
 *
 *	This class is referenced whenever a corpse is created, used, or assessed.
 *
 *	Author: Cubem0j0
 */

#include "Client.h"
#include "MasterServer.h"
#include "Object.h"
#include "WorldManager.h"
#include "TreasureGen.h"

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of corpses.
 *
 *	Called whenever a corpses object should be initialized.
 */
cCorpse::cCorpse( WORD type, DWORD dwGUID, cLocation& Loc, char *szName, char *szDesc)
{
	//SetLocation( pcLoc );
	m_dwGUID = dwGUID;//cWorldManager::NewGUID_Object( );
	m_strName.assign( szName );
	m_strDescription.assign( szDesc );
	m_dwModel = type;
	m_dwType = 0;
	m_fStatic = false;
	m_dwDoorState = 0x0CL;
	CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	//Set the objects state
	this->SetState(0);
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of a corpse in the world.
 *
 *	This function is called whenever a corpse should be created in the world for a client.
 *	@return cMessage - Returns a server message to the client.
 */
cMessage cCorpse::CreatePacket( )
{
	cMessage cmCreate;
	unsigned char bUnknown[] = {
	0x00, 0x00, 0x3D, 0x00, 0x02, 0xF0, 0xB5, 0x02, 0x11, 0x00, 0x00, 0x00,
	};

	cModels *pcMonster = cModels::FindModel(this->m_MonsterModelID);

	#ifdef _DEBUG
	cMasterServer::ServerMessage( ColorYellow,NULL,"m_dwModel = %d",this->m_MonsterModelID);
	#endif

	if(pcMonster)
	{
		cmCreate	<< 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
		cmCreate	<< pcMonster->m_bPaletteChange
					<< pcMonster->m_bTextureChange
					<< pcMonster->m_bModelChange;

		// The Model Vectors
		if ( pcMonster->m_bPaletteChange != 0) 
		{
			for (int i = 0; i < pcMonster->m_bPaletteChange; i++)
			{
				cmCreate.pasteData((UCHAR*)&pcMonster->m_vectorPal[i],sizeof(pcMonster->m_vectorPal[i]));
			}
		}
		
		if (pcMonster->m_bPaletteChange != 0) 
		{
			cmCreate << WORD( pcMonster->m_wUnknown1 );
		}
		
		if ( pcMonster->m_bTextureChange != 0) 
		{
			for (int i = 0; i < pcMonster->m_bTextureChange; i++)
			{
				cmCreate.pasteData((UCHAR*)&pcMonster->m_vectorTex[i],sizeof(pcMonster->m_vectorTex[i]));
			}
		}

		if ( pcMonster->m_bModelChange != 0) 
		{
			for (int i = 0; i < pcMonster->m_bModelChange; i++)
			{
				cmCreate.pasteData((UCHAR*)&pcMonster->m_vectorMod[i],sizeof(pcMonster->m_vectorMod[i]));
			}
		}
	}

	cmCreate.pasteAlign(4);
	DWORD dwFlags = 0x00019883L;

	cmCreate << dwFlags;
	cmCreate << WORD( 0x0414 );
	cmCreate << WORD( 0x000C );

	// MASK 0x00010000 unknown Bytes - Starting Animation  
	cmCreate << 0x0CL;
	cmCreate.pasteData(bUnknown,sizeof(bUnknown));
	cmCreate << 0x0L;
	// MASK 0x00008000 -- Location 
	if ( !m_fIsOwned )
		cmCreate.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );
	// MASK 0x00000002 -- Animation Set
	DWORD dwAnimC = 0x09000000L + m_wAnimConfig;
	cmCreate << dwAnimC;
	// MASK 0x00000800 -- Sound Set 
	DWORD dwSoundSet = 0x20000000L + m_wSoundSet;
	cmCreate << dwSoundSet;
	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	cmCreate << 0x3400006EL;//0x00000000L;
	// MASK 0x00000001 -- ModelNumber
	DWORD dwModel = 0x02000000L + m_dwModel;
	cmCreate << dwModel;
	
	cmCreate << float(m_flScale);

	// SeaGreens
	WORD wNuminteracts = 0x0;
	WORD wNumbubbles = 0x0;
	WORD wNumJumps = 0x0;
	WORD wNumOverrides = 0x0;
	WORD wUnkFlag8 = 0x0;
	WORD wUnkFlag10 = 0x0;
	
	cmCreate	<< m_wPositionSequence
				<< m_wNumAnims //wNuminteracts 
				<< wNumbubbles 
				<< wNumJumps 
				<< m_wNumPortals 
				<< m_wNumAnims 
				<< wNumOverrides 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;

	DWORD dwFlags2 = 0x00200036;
	
	cmCreate << dwFlags2 << m_strName.c_str( ) << WORD(m_wModel) << WORD(m_wIcon);//WORD( 0x019C ) << WORD( 0x1317 );;//WORD( 0x0116 ) << WORD( 0x1317 );
				
	/* Category of object:
		0x00000001 Melee Weapon 
		0x00000002 Armor 
		0x00000004 Clothing 
		0x00000008 Jewelry 
		0x00000010 Creature (Player/NPC/Monster) 
		0x00000020 Food 
		0x00000040 Pyreals 
		0x00000080 Miscellaneous 
		0x00000100 Missile Weapons/Ammunition 
		0x00000200 Containers 
		0x00000400 Wrapped Fletching Supplies, House Decorations 
		0x00000800 Gems, Pack dolls, Decorative Statues 
		0x00001000 Spell Components 
		0x00002000 Books, Parchment, Scrolls, Signs, Statues 
		0x00004000 Keys, Lockpicks 
		0x00008000 Casting Item (wand, orb, staff) 
		0x00010000 Portal 
		0x00020000 Lockable 
		0x00040000 Trade Notes 
		0x00080000 Mana Stones, Mana Charges 
		0x00100000 Services 
		0x00200000 unknown (no longer plants) 
		0x00400000 Cooking Ingredients and Supplies, Plants, Dye Pots 
		0x00800000 Loose Fletching Supplies 
		0x01000000 unknown 
		0x02000000 unknown 
		0x04000000 Alchemy Ingredients and Supplies, Oils, Dye Vials 
		0x08000000 unknown 
		0x10000000 Lifestone 
		0x20000000 Ust 
		0x40000000 Salvage 
		0x80000000 unknown 
	*/

	DWORD dwObjectFlags1 = 0x0200L;	
	cmCreate << dwObjectFlags1;

	/* Behavior of object:
		0x00000001 can be opened (false if locked) 
		0x00000002 can be inscribed 
		0x00000004 cannot be picked up 
		0x00000008 is a player 
		0x00000010 is not an npc 
		0x00000020 unknown 
		0x00000040 unknown 
		0x00000080 cannot be selected 
		0x00000100 can be read 
		0x00000200 is a merchant 
		0x00000400 is a pk altar 
		0x00000800 is an npk altar 
		0x00001000 is a door 
		0x00002000 is a corpse 
		0x00004000 can be attuned to (lifestone) 
		0x00008000 adds to health, stamina or mana 
		0x00010000 is a healing kit 
		0x00020000 is a lockpick 
		0x00040000 is a portal 
		0x00800000 is a foci 
		0x04000000 has an extra flags DWORD 
	*/
	DWORD dwObjectFlags2 = 0x2015L;
	cmCreate << dwObjectFlags2;


	/* Masked against dwFlags2 in reverse order:
		0x00000002 - Item Slots
		0x00000004 - Pack Slots
		0x00000008 - Value
		0x00000010 - Unknown1
		0x00000020 - Approach Distance
		0x00200000 - Burden
	*/
	cmCreate << BYTE (0x78); //78 is item slots. (120)
	cmCreate << BYTE (0x00); //00 is pack slots
	//cmCreate << DWORD (0x000009C4); //Value
	cmCreate << DWORD (0x00000030); //Unknown
	cmCreate << float(3.0); //Approach Distance.
	cmCreate << WORD (0x0000); //Burden
	
	return cmCreate;
}

/**
 *	Handles the actions of corpse objects.
 *
 *	This function is called whenever a corpse is used.
 */
void cCorpse::Action(cClient* who)
{

//Logic for open/close
switch (GetState())
{
	case 0:
		{
/*
			char szCommand[100];
			RETCODE retcode;
			
			int ItemType;
			DWORD dwItemModelID;
			int test;
			test = 70;
			
			sprintf( szCommand, "SELECT * FROM monsters_loot where dwModelID = %d;", this->GetType());
			#ifdef _DEBUG
			cMasterServer::ServerMessage( ColorYellow,NULL,"This GUID: %d, This Monster Model: %d",this->GetGUID(),this->GetMonsterModelID());
			#endif
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLExecute( cDatabase::m_hStmt );
			int iCol = 3;
			
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &ItemType, sizeof( INT ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwItemModelID, sizeof( DWORD ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			who->m_pcAvatar->m_CorpseTarget = this->GetGUID();

			cMessage cmSetPackContents;
			cmSetPackContents << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x0196L << m_dwGUID;
			for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; i++ )
			{
				//Cubem0j0:  We find the model number so we can get some vars.
				cItemModels *pcModel = cItemModels::FindModel(dwItemModelID);
				
				switch(ItemType)
				{
					case 1:
					{
					cWeapon* aWeapon = new cWeapon(cWorldManager::NewGUID_Object(),this->GetGUID(),dwItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wBurden,pcModel->m_dwValue,pcModel->m_bWieldType);
					who->AddPacket( WORLD_SERVER, aWeapon->CreatePacketContainer(this->GetGUID(),pcModel->m_dwModelID),3);
					this->AddInventory(aWeapon);
					cmSetPackContents << 0x01L << aWeapon->GetGUID() << 0x0L;				
					#ifdef _DEBUG
					cMasterServer::ServerMessage( ColorYellow,NULL,"Case 1");
					#endif
					break;
					}
					
					case 2:
					{
					cFood* aFood = new cFood(cWorldManager::NewGUID_Object(),this->GetGUID(),dwItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wBurden,pcModel->m_dwValue);
					who->AddPacket( WORLD_SERVER, aFood->CreatePacketContainer(this->GetGUID(),pcModel->m_dwModelID),3);
					this->AddInventory(aFood);
					cmSetPackContents << aFood->GetGUID() << 0x0L;
					break;
					}
		
					case 3:
					{
					cArmor* aArmor = new cArmor(cWorldManager::NewGUID_Object(),this->GetGUID(),dwItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
					who->AddPacket( WORLD_SERVER, aArmor->CreatePacketContainer(this->GetGUID(),pcModel->m_dwModelID),3);
					this->AddInventory(aArmor);
					cmSetPackContents << aArmor->GetGUID() << 0x0L;
					break;
					}
		
					case 8:
					{
					cWands* aWand = new cWands(cWorldManager::NewGUID_Object(),this->GetGUID(),dwItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
					who->AddPacket( WORLD_SERVER, aWand->CreatePacketContainer(this->GetGUID(),pcModel->m_dwModelID),3);
					this->AddInventory(aWand);
					cmSetPackContents << aWand->GetGUID() << 0x0L;
					break;
					}
		
					case 13:
					{
					cSpellComps* aReg = new cSpellComps(cWorldManager::NewGUID_Object(),this->GetGUID(),dwItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
					who->AddPacket( WORLD_SERVER, aReg->CreatePacketContainer(this->GetGUID(),pcModel->m_dwModelID),3);
					this->AddInventory(aReg);
					//cmSetPackContents << 0x01L << aReg->GetGUID() << 0x0L;
					break;
					}
					
				}
			who->AddPacket(WORLD_SERVER,cmSetPackContents,4);
			}
  */
			//I just explicitly use a model ID here, I will change this to a function later.
			cItemModels *pcModel = cItemModels::FindModel(147);

			//Use the Item Container classes to create the item in a corpse/chest.
			cLockpicks* Lockpicks = new cLockpicks(cWorldManager::NewGUID_Object(),this->GetGUID(),pcModel->m_dwModelNumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wUses,pcModel->m_wUseLimit);

			//Need a way for the cObject class to find the Item Model ID
			Lockpicks->m_dwItemModelID = pcModel->m_dwModelID;

			//Send the create packet
			who->AddPacket(WORLD_SERVER,Lockpicks->CreatePacketContainer(this->GetGUID(),Lockpicks->GetItemModelID()),3);
			
			#ifdef _DEBUG
			cMasterServer::ServerMessage( ColorYellow,NULL,"ItemModelID is: %d",Lockpicks->GetItemModelID());
			#endif

			//Add the item to the corpse inventory.
			this->AddInventory(Lockpicks);
				
			//SQL Done
//			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
//			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
				
			//Set the GUID of the corpse on the avatar (this is used in find the item in the corpses inventory)
			who->m_pcAvatar->m_CorpseTarget = this->GetGUID();

			if(this->FindInventory(Lockpicks->GetGUID())) 
			{
				#ifdef _DEBUG
				cMasterServer::ServerMessage( ColorYellow,NULL,"Item Found");
				#endif
			}

			//Display the item slots in the chest (0x0196)
			cMessage cmSetPackContents;
			cmSetPackContents << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x0196L << m_dwGUID
				<< 0x01L << Lockpicks->GetGUID() << 0x0L;
			who->AddPacket(WORLD_SERVER,cmSetPackContents,4);

			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
			who->AddPacket(WORLD_SERVER,cmActionComplete,4);
				
			this->SetState(1);
			break;
		}
case 1:
		{
			#ifdef _DEBUG
			cMasterServer::ServerMessage(ColorYellow,NULL,"m_wState is %d",m_wState);
			#endif

			//Close Container
			cMessage cmCloseContainer;
			cmCloseContainer << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x0052L << m_dwGUID;
			who->AddPacket(WORLD_SERVER,cmCloseContainer,4);

			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
			who->AddPacket(WORLD_SERVER,cmActionComplete,4);
			this->SetState(0);
			break;
		}
	}
}

/**
 *	Handles the assessment of corpse objects.
 *
 *	This function is called whenever a corpse is assessed by a client.
 *	Returns a server message to the client.
 */
void cCorpse::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	DWORD flags = 0x00000009;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags 
	<< 0x01L						//Success = 0x01, Failure = 0x00
	<< WORD(0x0002)
	<< WORD(0x0010)
	<< 0x13L						//Value
	<< 0x00L						//Corpse always zero.
	<< 0x05L						//Burden
	<< 0xCCL						//for now, static amount it eventuall should add up all the item bu's

	<< WORD(0x0001)					//Count (as above) total number of DWORDS
	<< WORD(0x0008)					//Unknown
	<< 0x10L
	<< this->m_strDescription.c_str();	//This is the "Killed by" message when ID'd.
	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);

}