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
 *	@file cNPC.cpp
 *	Implements general functionality for Non-Player Characters (NPCs).
 *
 *	Functionality includes giving, buying, and sell items to vendors.
 *
 *	Inherits from the cObject class.
 */

#include "object.h"
#include "masterserver.h"
#include "worldmanager.h"
#include "cNPCModels.h"
#include <algorithm>
    
/***************
 *	constructors
 **************/

/**
 *	Handles the creation of NPCs.
 *
 *	Called whenever an NPC object should be initialized.
 */
cNPC::cNPC(DWORD dwGUID, DWORD dwNPCModelID, char *szName, WORD wGender, cLocation *pcLoc, DWORD dwSellCategories )
: m_dwNPCModelID ( dwNPCModelID )
{
	SetLocation( pcLoc );
//	m_dwGUID = cWorldManager::NewGUID_Object( );
//  Cubem0j0: Implement NPC IDs
	m_dwGUID = dwGUID;
	m_dwNPCModelID = dwNPCModelID;
	m_strName.assign( szName );
	m_wGender = wGender;
	m_fStatic = TRUE;
	m_NPCStats.m_dwLevel = 10;
	m_dwSellCategories = dwSellCategories;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of NPCs.
 *
 *	This function is called whenever an NPC should be created for a client.
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cNPC::CreatePacket()
{
	cMessage cmReturn;
	cNPCModels *pcModel = cNPCModels::FindModel( m_dwNPCModelID );
	DWORD dwFlags1;

	if (pcModel) 
	{
		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
		cmReturn	<< pcModel->m_bPaletteChange
					<< pcModel->m_bTextureChange
					<< pcModel->m_bModelChange;
		

		
		if ( pcModel->m_bPaletteChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bPaletteChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorPal[i],sizeof(pcModel->m_vectorPal[i]));
			}
		}

		cmReturn << pcModel->m_wPaletteCode;
		
		if ( pcModel->m_bTextureChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bTextureChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorTex[i],sizeof(pcModel->m_vectorTex[i]));
			}
		}
		if ( pcModel->m_bModelChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bModelChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorMod[i],sizeof(pcModel->m_vectorMod[i]));
			}
		}

		//TODO:  Add support for "Monster" NPCs (Mayor Ko Ko, etc)
		cmReturn.pasteAlign(4);
			
				dwFlags1 = 0x00018803;
				cmReturn << dwFlags1 << WORD(0x0418);
		
		//Cubem0j0: This unknown appears to deal with whether or not the NPC is a merchant, quest npc, or just a regular npc.
		switch(IsVendor)
		{
			default:
				{
					cmReturn << WORD(0x0040);	// Unknown
					break;
				}
			case 1:
				{
					cmReturn << WORD(0x0020);  // Merchant
					break;
				}
			case 2: 
				{
					cmReturn << WORD(0x0060);  // Quest NPC
					break;
				}
		}
		
		DWORD dwNumInitAnimationBytes = 8;
		cmReturn << dwNumInitAnimationBytes;
				
		BYTE ucInitialAnimation[] =  //states which animation the model starts out in
		{0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00};
				
		for(int blah = 0;blah<sizeof(ucInitialAnimation);blah++)
			cmReturn << ucInitialAnimation[blah];
				
		cmReturn << 0x00000000L;	// Unknown DWORD
			
		cmReturn.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) );	//Next comes the location
		cmReturn << DWORD (0x09000001);
		cmReturn << DWORD (0x20000001);
		cmReturn << 0x02000000 + pcModel->m_dwModelNumber;


		// SeaGreens
		WORD wUnkFlag2 = 0x1;
		WORD wUnkFlag3 = 0x1;
		WORD wUnkFlag4 = 0;
		WORD wUnkFlag6 = 0;
		WORD wUnkFlag7 = 0;
		WORD wUnkFlag8 = 0;
		WORD wUnkFlag10 = 0;

		cmReturn	<< m_wPositionSequence //movement
					<< wUnkFlag2 // animations
					<< wUnkFlag3 // bubble modes
					<< wUnkFlag4 //num jumps
					<< m_wNumPortals 
					<< wUnkFlag6  //anim count
					<< wUnkFlag7  // overrides
					<< wUnkFlag8
					<< m_wNumLogins
					<< wUnkFlag10;
		
		DWORD dwFlags2;
		//vendor or not
		switch(IsVendor)
		{
		case 0:
			dwFlags2 = 0x00800016;
			break;
		case 1:
			dwFlags2 = 0x00800036;
			break;
		case 2:
			dwFlags2 = 0x00900036;
			break;
			//Town Criers...
		case 3:
			dwFlags2 = 0x00900036;
			break;
		default:
			dwFlags2 = 0x00800016;
			break;
		}

		cmReturn << dwFlags2;			// Word 0080 - Flag Word 0016 Length of Name
		
		cmReturn << Name( );	// Object's Name

		cmReturn << pcModel->m_wModel << pcModel->m_wIcon;

		DWORD dwObjectFlags1;
		DWORD dwObjectFlags2;

		switch(IsVendor)
		{
			case 0:
				{
					dwObjectFlags1 = 0x00000010;
					dwObjectFlags2 = 0x00000004;
					break;
				}
			case 1:
				{
					dwObjectFlags1 = 0x00000010;
					dwObjectFlags2 = 0x00000204;
					break;
				}
			case 2:
				{
					dwObjectFlags1 = 0x00000010;
					dwObjectFlags2 = 0x00000004;
					break;
				}
			default:
				{
					dwObjectFlags1 = 0x00000010;
					dwObjectFlags2 = 0x00000004;
					break;
				}
		}

		cmReturn << dwObjectFlags1 << dwObjectFlags2;
		BYTE bNumberOfSlots = 0xFF; 
				cmReturn << bNumberOfSlots;
		BYTE bNumPackSlots = 0xFF; 
				cmReturn << bNumPackSlots;
		cmReturn << 0x20L;
		
		if(dwFlags2 & 0x00000020)
		{
			cmReturn << float(3.0);
		}

		//Cubem0j0:  0x00800000 - This flag seems to affect what options you have when selecting the object, and also
		//			 how it appears on radar.

		switch(IsVendor)
		{
			case 1:
				{
					DWORD dwUnknown4 = 0x00020004;
					cmReturn << dwUnknown4;
					break;
				}
			
			default:
				{
					DWORD dwUnknown4 = 0x00010408;
					cmReturn << dwUnknown4;
					break;
				}
		}
	}
	else  // nothing
	{
		cmReturn << 0xF755L << DWORD(1) << DWORD(124) << 0x3F2B4E94L;
	}

	return cmReturn;
}

/**
 *	Handles the actions of NPC objects.
 *
 *	This function is called whenever an NPC is used or should perform an general action.
 *
 *	Author: Cubem0j0
 */
void cNPC::Action(cClient* who)
{
	float flHeading;
	flHeading = GetHeadingTarget( who->m_pcAvatar->m_Location.m_dwLandBlock, who->m_pcAvatar->m_Location.m_flX, who->m_pcAvatar->m_Location.m_flY, who->m_pcAvatar->m_Location.m_flZ );
	cWorldManager::SendToAllWithin( 5, m_Location, TurnToTarget(flHeading, who->m_pcAvatar->GetGUID( ) ), 3 );

	switch(m_dwMode)
	{
	case NPCMODE_SINGLE:
		{
		cMasterServer::ServerMessage(ColorBrown,who,"%s tells you, \"%s\"",m_strName.c_str(),vsMessages[0].c_str());
		break;
		}
	case NPCMODE_MULTI:
		{
		for(int a = 0;a<iNumMessages;a++)
		{
			cMasterServer::ServerMessage(ColorBrown,who,"%s tells you, \"%s\"",m_strName.c_str(),vsMessages[a].c_str());
		}
		break;
		}
	case NPCMODE_RANDOM:
		{
		int rnd = (double(rand())/RAND_MAX)*iNumMessages;
		if(rnd==iNumMessages) //not sure if this is needed, but rand() might equal RAND_MAX
			rnd--;
		cMasterServer::ServerMessage(ColorBrown,who,"%s tells you, \"%s\"",m_strName.c_str(),vsMessages[rnd].c_str());
		break;
		}
	}
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);

	if(GetIsVendor() == 1)
	{


			
		this->ApproachVendor(who);
	}
}

/**
 *	Handles the assessment of NPC objects.
 *
 *	This function is called whenever an NPC is assessed by a client.
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cNPC::Assess(cClient *pcAssesser)
{
	m_NPCStats.m_dwLevel = 10;
	m_NPCStats.m_dwStr = 10;
	m_NPCStats.m_dwEnd = 10;
	m_NPCStats.m_dwQuick = 10;
	m_NPCStats.m_dwCoord = 10;
	m_NPCStats.m_dwFocus = 10;
	m_NPCStats.m_dwSelf = 10;
	m_NPCStats.m_dwSpecies = 31;

	cMessage cmAssess;

	DWORD dwCharacterFlags; // Needs Decoding
	dwCharacterFlags = 0x0CL; // 0x04 - Level, Health | 0x08 - Stats

	cmAssess	<< 0xF7B0L 
				<< pcAssesser->m_pcAvatar->GetGUID( ) 
				<< ++pcAssesser->m_dwF7B0Sequence 
				<< 0xC9L 
				<< m_dwGUID 
				<< 0x100L // Flags 0x00000100 - Character Data
				<< 0x1L;  // Pass/Failed

	// Process the Flag Specified Data
	cmAssess	<< 	dwCharacterFlags
				// Mask 0x04
				<< m_NPCStats.m_dwLevel
				<< m_NPCStats.m_dwEnd/2
				<< m_NPCStats.m_dwEnd/2
				// Mask 0x08
				<< m_NPCStats.m_dwStr
				<< m_NPCStats.m_dwEnd
				<< m_NPCStats.m_dwQuick
				<< m_NPCStats.m_dwCoord
				<< m_NPCStats.m_dwFocus
				<< m_NPCStats.m_dwSelf
				<< m_NPCStats.m_dwEnd
				<< m_NPCStats.m_dwSelf
				<< m_NPCStats.m_dwEnd
				<< m_NPCStats.m_dwSelf
				// End Mask
				<< m_NPCStats.m_dwSpecies;

	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);
	
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the purchasing of items from NPC objects.
 *
 *	This function is called whenever a player buys an item from an NPC.
 *
 *	@param *pcClient - A pointer to the client whose avatar is purchasing the item.
 *	@param VenderID - A the GUID of the vendor.
 *	@param ItemID - The GUID of the object to buy.
 */
void cNPC::BuyItem(cClient *pcClient, DWORD VendorID, DWORD ItemID)
{
	DWORD adwGUID = pcClient->m_pcAvatar->GetGUID( );
	//Cube:  First, find the vendor:
	cNPC *vendor = reinterpret_cast<cNPC *>(cWorldManager::FindObject(VendorID));

	//Cube:  Now find the item in that vendor's inventory
	cObject *ObjectBuy = vendor->FindInventory(ItemID);

	//Cube:  Finally find the object Model ID
	cItemModels *pcModel = cItemModels::FindModel(ObjectBuy->GetItemModelID());

	
	switch(pcModel->m_ItemType)
	{
	case 1:	//Weapons
		{
			//Create the item on the avatar
			cWeapon* aWeapon = new cWeapon(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->m_dwWeaponDamage,pcModel->m_dwWeaponSpeed,pcModel->m_dwWeaponSkill,pcModel->m_dwDamageType,pcModel->m_dWeaponVariance,pcModel->m_dWeaponModifier,pcModel->m_dWeaponPower,pcModel->m_dWeaponAttack);
			pcClient->AddPacket( WORLD_SERVER, aWeapon->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aWeapon);

			//Reload the Vendors inventory
			this->ApproachVendor(pcClient);

			//Add the Item to the player's inventory.  (Don't even know that we really need this step...)
			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aWeapon->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );

			//Subtract the price of the item from total pyreals
			DWORD npy = pcClient->m_pcAvatar->m_dwPyreals - aWeapon->GetItemValue();
			cMessage cmRet;
			cmRet << 0x237L << m_bStatSequence++ << 0x14L << npy;
			pcClient->AddPacket( WORLD_SERVER, cmRet, 4);

			#if _DEBUG
			cMasterServer::ServerMessage(ColorYellow,NULL,"Item value is: %d",aWeapon->GetItemValue());
			#endif

			//Play the "buy" sound
			cMessage Sound = pcClient->m_pcAvatar->SoundEffect(122);

			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}

	case 2:	//Food
		{
			cFood* aFood = new cFood(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wStack, pcModel->m_wBurden, pcModel->m_dwVitalID, pcModel->m_vital_affect);
			pcClient->AddPacket( WORLD_SERVER, aFood->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aFood);

			this->ApproachVendor(pcClient);

			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aFood->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );


			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);
			break;
		}

	case 3:	//Armor
		{
			cArmor* aArmor = new cArmor(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_dwArmor_Level,pcModel->m_fProt_Slashing, pcModel->m_fProt_Piercing, pcModel->m_fProt_Bludgeon, pcModel->m_fProt_Fire, pcModel->m_fProt_Cold, pcModel->m_fProt_Acid, pcModel->m_fProt_Electric);
			pcClient->AddPacket( WORLD_SERVER, aArmor->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aArmor);

			this->ApproachVendor(pcClient);

			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aArmor->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );


			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
		
	case 5:	//Scrolls
		{
			cScrolls* aScrolls = new cScrolls(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
			pcClient->AddPacket( WORLD_SERVER, aScrolls->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aScrolls);

			this->ApproachVendor(pcClient);

			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aScrolls->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );


			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
	case 7:	//Lockpicks
		{
			cLockpicks* aPick = new cLockpicks(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wUses,pcModel->m_wUseLimit);
			pcClient->AddPacket( WORLD_SERVER, aPick->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aPick);

			this->ApproachVendor(pcClient);

			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aPick->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );


			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
	case 8:	//Wands
		{
			cWands* aWand = new cWands(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
			pcClient->AddPacket( WORLD_SERVER, aWand->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aWand);

			this->ApproachVendor(pcClient);

			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aWand->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );


			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
	case 12:
		{
			cShield* aShield = new cShield(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_dwArmor_Level);
			pcClient->AddPacket( WORLD_SERVER, aShield->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aShield);
			
			this->ApproachVendor(pcClient);
			
			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aShield->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );
			
			//Done
			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
	case 13:
		{
			cSpellComps* aReg = new cSpellComps(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,1,pcModel->m_wBurden);
			pcClient->AddPacket( WORLD_SERVER, aReg->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aReg);
			
			this->ApproachVendor(pcClient);
			
			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aReg->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );
			
			//Done
			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}

	case 14:
		{
			cGems* aGem = new cGems(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
			pcClient->AddPacket( WORLD_SERVER, aGem->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aGem);
			
			this->ApproachVendor(pcClient);
			
			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aGem->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );
			
			//Done
			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
	case 18:
		{
			cClothes* aShirt = new cClothes(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
			pcClient->AddPacket( WORLD_SERVER, aShirt->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aShirt);
			
			this->ApproachVendor(pcClient);
			
			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aShirt->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );
			
			//Done
			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
	case 19:
		{
			cJewelry* aRing = new cJewelry(cWorldManager::NewGUID_Object(),adwGUID,ObjectBuy->GetItemModelID(),1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
			pcClient->AddPacket( WORLD_SERVER, aRing->CreatePacketContainer(adwGUID,pcModel->m_dwModelID),3);
			pcClient->m_pcAvatar->AddInventory(aRing);
			
			this->ApproachVendor(pcClient);
			
			cMessage cmInsInv;
	 		cmInsInv << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << aRing->GetGUID() << pcClient->m_pcAvatar->GetGUID( ) << 0L << 0L;
			pcClient->AddPacket( WORLD_SERVER, cmInsInv, 4 );
			
			//Done
			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
			pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
	}
	
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles trade interaction with vendor NPCs.
 *
 *	This function is called whenever a player approaches a vendor.
 *
 *	@param *who - A pointer to the client whose avatar approaching the vendor.
 */
void cNPC::ApproachVendor(cClient *who)
{
	if(GetIsVendor() == 1)
	{
		cMasterServer::ServerMessage(ColorYellow,NULL,"inven size is: %d",this->m_NPC_lstInventory.size());
		if (this->m_NPC_lstInventory.size() == 0)
		{
		//If inventory size is zero, nothing will happen.

		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
		who->AddPacket(WORLD_SERVER,cmActionComplete,4);
		}
		else
		{
		//Cubem0j0:  Vendor Stuff
		DWORD BuyCategories = 0x000cb000; //0x000410A0;  //Will figure these out later.
		DWORD unk1 = 0x00000000;
		DWORD HighestBuy = 0x000186A0;	//0x000186A0; //100,000.
		DWORD unk2 = 0x00000001;
		float BuyRate = 0.9f;		//Corrected values
		float SellRate = 1.55f;		//Corrected values
		DWORD Inventory = 0xFFFFFFFF;

		/*	OLD VARS
		DWORD ItemCount = 0x0002L;//0012
		DWORD Item_ID = 0x90873824;  //number is odd, anything lower causes crash.
		DWORD flags = 0x10207019;	 //add 1 if item stacks.
		char* name = "Health Potion";
		WORD Model = 0x0179;
		WORD Icon = 0x1D72;
		DWORD Sell_Categories = 0x00000100;
		*/

		cMessage cmApproachVendor;
		cmApproachVendor << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x0062L 
		<< m_dwGUID						//Vendor GUID
		<< BuyCategories
		<< unk1
		<< HighestBuy					//Highest Buy? (unconfirmed)
		<< unk2
		<< BuyRate						//Buy Rate (unconfirmed)
		<< SellRate						//Sell Rate (Unconfirmed)
		<< this->m_NPC_lstInventory.size();	//Does not appear to be item count

		//Loop through inventory and find all items.				
		for (int i = 0; i < this->m_NPC_lstInventory.size(); i++)
			{
				//Find each item by it's GUID
				cObject *v_item = this->FindInventory(this->v_guids[i]);

				//Associate model with item
				cItemModels *v_model = cItemModels::FindModel(v_item->GetItemModelID());
			
				//The v_model->m_dwObjectFlags1 will determine the sell categories for the vendor.
				//So we add logic to send the correctly formatted message
				//Note:  * are sections that are complete.
				switch(v_model->m_dwObjectFlags1)
				{
					case 0x00000001:		//Weapons (NOTE: Vendor works, Corpse works, GS works)
					{
				cmApproachVendor 
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< WORD(0xFFFF)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10214218
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10 - use Unknown_v2 from database
					<< v_model->m_bWieldType		//Wield type (Byte, padded)
					<< m_dwGUID						//Container ID
					<< DWORD(0x100000)//v_model->m_dwEquipPossible	//EquipMask :the potential equipment slots this object may be placed in 
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< v_model->m_wHooks			//Hook Type (WORD, no padding)
					//Below is padding DO NOT REMOVE!
					<< WORD(0x0000)				
					<< BYTE(0x00);
				//cMasterServer::ServerMessage(ColorYellow,NULL,"Value is : %d",v_model->m_dwValue);
					break;
					}
					case 0x00000002:		//Armor	(NOTE: Vendor works, Corpse works, GS works)
					{

				cmApproachVendor 
					<< WORD(0xFFFF)
					<< WORD(0xFFFF)
					<< v_item->GetGUID()		
					<< v_model->m_dwFlags2		
					<< v_item->Name()
					<< v_model->m_wModel		
					<< v_model->m_wIcon		
					<< v_model->m_dwObjectFlags1	
					<< v_model->m_dwObjectFlags2;	
					if(v_model->m_ItemType == 3)
					{
						if(v_model->m_dwFlags2 & 0x00000008)
						{
							cmApproachVendor
								<< v_model->m_dwValue;	
						}
						if(v_model->m_dwFlags2 & 0x00000010)
						{
							cmApproachVendor
								<< v_model->m_dwUnknown_v2;
						}
						if(v_model->m_dwFlags2 & 0x00004000)
						{
							cmApproachVendor
								<< m_dwGUID;
						}
						if(v_model->m_dwFlags2 & 0x00010000)
						{
							cmApproachVendor
								<< v_model->m_dwEquipPossible; 
						}
						if(v_model->m_dwFlags2 & 0x00040000)
						{
							cmApproachVendor
								<< v_model->m_dwCoverage;
						}
						if(v_model->m_dwFlags2 & 0x00200000)
						{
							cmApproachVendor	
								<< v_model->m_wBurden;
						}
						if(v_model->m_dwFlags2 & 0x10000000)
						{
							cmApproachVendor
								<< v_model->m_wHooks;
						}
						
						else
						{
							cmApproachVendor
								<< WORD(0x0000);
						}
					}

					if(v_model->m_ItemType == 12)
					{
						if(v_model->m_dwFlags2 & 0x00000008)
							cmApproachVendor << v_model->m_dwValue;

						if(v_model->m_dwFlags2 & 0x00000010)						
							cmApproachVendor << v_model->m_dwUnknown_v2;
						
						if(v_model->m_dwFlags2 & 0x00000080)						
							cmApproachVendor << v_model->m_dwIconHighlight;
						
						if(v_model->m_dwFlags2 & 0x00000200)						
							cmApproachVendor << BYTE(0x04);
						
						if(v_model->m_dwFlags2 & 0x00004000)						
							cmApproachVendor << m_dwGUID;
						
						if(v_model->m_dwFlags2 & 0x00010000)						
							cmApproachVendor << v_model->m_dwEquipPossible;
						
						if(v_model->m_dwFlags2 & 0x00200000)						
							cmApproachVendor << v_model->m_wBurden;
						
						if(v_model->m_dwFlags2 & 0x01000000)						
							cmApproachVendor << v_model->m_fWorkmanship;
						
						if(v_model->m_dwFlags2 & 0x10000000)						
							cmApproachVendor << v_model->m_wHooks;
						
						if(v_model->m_dwFlags2 & 0x80000000)
							cmApproachVendor << v_model->m_dwMaterialType;
						else
						{
						cmApproachVendor	<< WORD(0x0000)
											<< BYTE(0x00);
						}
					}
					break;

					}	

					case 0x00000004:		//Clothes
					{
				cmApproachVendor 
					<< WORD(0xFFFF)
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2;	//Object flags 2
					if(v_model->m_dwFlags2 & 0x00000008)
						cmApproachVendor << v_model->m_dwValue;
					if(v_model->m_dwFlags2 & 0x00000010)
						cmApproachVendor << v_model->m_dwUnknown_v2;
					if(v_model->m_dwFlags2 & 0x00000080)
						cmApproachVendor << v_model->m_dwIconHighlight;
					if(v_model->m_dwFlags2 & 0x00004000)
						cmApproachVendor << m_dwGUID;
					if(v_model->m_dwFlags2 & 0x00010000)
						cmApproachVendor << v_model->m_dwEquipPossible;
					if(v_model->m_dwFlags2 & 0x00040000)
						cmApproachVendor << v_model->m_dwCoverage;
					if(v_model->m_dwFlags2 & 0x00200000)
						cmApproachVendor << v_model->m_wBurden;
					if(v_model->m_dwFlags2 & 0x01000000)
						cmApproachVendor << v_model->m_fWorkmanship;
					if(v_model->m_dwFlags2 & 0x10000000)
						{
							cmApproachVendor
								<< v_model->m_wHooks;
						}
						
					else
						{
							cmApproachVendor
								<< WORD(0x0000);
						}

					if(v_model->m_dwFlags2 & 0x80000000)
						cmApproachVendor << v_model->m_dwMaterialType;
					break;
					}		
					case 0x00000008:		//Jewelry
					{
				cmApproachVendor 
					<< WORD(0xFFFF)
					<< WORD(0xFFFF)			//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()		//Item GUID
					<< v_model->m_dwFlags2		//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel		//Model
					<< v_model->m_wIcon		//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2;	//Object flags 2
					if(v_model->m_dwFlags2 & 0x00000008)
						cmApproachVendor << v_model->m_dwValue;
					if(v_model->m_dwFlags2 & 0x00000010)
						cmApproachVendor << v_model->m_dwUnknown_v2;
					if(v_model->m_dwFlags2 & 0x00000080)
						cmApproachVendor << v_model->m_dwIconHighlight;
					if(v_model->m_dwFlags2 & 0x00004000)
						cmApproachVendor << m_dwGUID;
					if(v_model->m_dwFlags2 & 0x00010000)
						cmApproachVendor << v_model->m_dwEquipPossible;
					if(v_model->m_dwFlags2 & 0x00200000)
						cmApproachVendor << v_model->m_wBurden;
					if(v_model->m_dwFlags2 & 0x01000000)
						cmApproachVendor << v_model->m_fWorkmanship;
					if(v_model->m_dwFlags2 & 0x10000000)
						{
							cmApproachVendor
								<< v_model->m_wHooks;
						}
						
					//else
					//	{
					//		cmApproachVendor
					//			<< WORD(0x0000);
					//	}
					if(v_model->m_dwFlags2 & 0x80000000)
						cmApproachVendor << v_model->m_dwMaterialType;
					cmApproachVendor
								<< WORD(0x0000);
					break;
					}	
					case 0x00000020:		//Food*
					{
				cmApproachVendor 
					<< WORD(0xFFFF)
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//00207018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< v_model->m_wStack			//NUmber of items in stack 
					<< v_model->m_wStackLimit		//Stack limit
					<< m_dwGUID						//Container ID
					<< v_model->m_wBurden;			//Burden (WORD, no padding)
					break;
					}	
					case 0x00000080:		//Misc (may need to define further)
					{
				cmApproachVendor 
					<< WORD(0xFFFF)
					<< WORD(0xFFFF)
					<< v_item->GetGUID()	
					<< v_model->m_dwFlags2		
					<< v_model->m_strName.c_str()	
					<< v_model->m_wModel		
					<< v_model->m_wIcon		
					<< v_model->m_dwObjectFlags1	
					<< v_model->m_dwObjectFlags2;	
					if(v_model->m_dwFlags2 & 0x00000008)
						cmApproachVendor << v_model->m_dwValue;
					if(v_model->m_dwFlags2 & 0x00000010)
						cmApproachVendor << v_model->m_dwUnknown_v2;
					if(v_model->m_dwFlags2 & 0x00000400)
						cmApproachVendor << v_model->m_wUses;
					if(v_model->m_dwFlags2 & 0x00000800)
						cmApproachVendor << v_model->m_wUseLimit;
					if(v_model->m_dwFlags2 & 0x00001000)
						cmApproachVendor << v_model->m_wStack;
					if(v_model->m_dwFlags2 & 0x00002000)
						cmApproachVendor << v_model->m_wStackLimit;
					if(v_model->m_dwFlags2 & 0x00004000)
						cmApproachVendor << m_dwGUID;
					if(v_model->m_dwFlags2 & 0x00200000)
						cmApproachVendor << v_model->m_wBurden;
					if(v_model->m_dwFlags2 & 0x10000000)
						cmApproachVendor << v_model->m_wHooks;	
					else
					{
						cmApproachVendor << WORD(0x0000);
					}
					break;
					}		
					case 0x00000100:		//Missile Weapons/Ammunition
					{
				cmApproachVendor 
					<< WORD(0xFFFF)
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2;	//Object flags 2

					//Cube:  NOTE: For ammo, ammo type comes before value
						if(v_model->m_dwFlags2 & 0x00000100)
						{
							cmApproachVendor
								<< v_model->m_wAmmoType;	
						}
						if(v_model->m_dwFlags2 & 0x00000008)
						{
							cmApproachVendor
								<< v_model->m_dwValue;	
						}
						if(v_model->m_dwFlags2 & 0x00000010)
						{
							cmApproachVendor
								<< v_model->m_dwUnknown_v2;
						}

						if(v_model->m_dwFlags2 & 0x00000080)
							cmApproachVendor << v_model->m_dwIconHighlight;

						if(v_model->m_dwFlags2 & 0x00000200)
						{
							cmApproachVendor
								<< v_model->m_bWieldType;
						}
						if(v_model->m_dwFlags2 & 0x00001000)
						{
							cmApproachVendor
								<< v_model->m_wStack;
						}
						if(v_model->m_dwFlags2 & 0x00002000)
						{
							cmApproachVendor
								<< v_model->m_wStackLimit;
						}
						if(v_model->m_dwFlags2 & 0x00004000)
						{
							cmApproachVendor
								<< m_dwGUID;
						}
						if(v_model->m_dwFlags2 & 0x00010000)
						{
							cmApproachVendor
								<< v_model->m_dwEquipPossible;
						}
						if(v_model->m_dwFlags2 & 0x00200000)
						{
							cmApproachVendor	
								<< v_model->m_wBurden;
						}
						if(v_model->m_dwFlags2 & 0x10000000)
						{
							cmApproachVendor
								<< v_model->m_wHooks;
						}
					//Below is padding DO NOT REMOVE!
				cmApproachVendor
					<< WORD(0x0000)				
					<< BYTE(0x00);
					break;
					}			
					case 0x00000200:		//Containers
					{
				cmApproachVendor 
					<< Inventory					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< m_dwGUID						//Container ID
					<< v_model->m_dwEquipPossible	//EquipMask :the potential equipment slots this object may be placed in 
					<< v_model->m_dwCoverage		//Coverage
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< v_model->m_wHooks;			//Hook Type (WORD, no padding)
					break;
					}	
					case 0x00000400:		//Fletching Supplies, Decorations
					{
				cmApproachVendor 
					<< Inventory					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< m_dwGUID						//Container ID
					<< v_model->m_dwEquipPossible	//EquipMask :the potential equipment slots this object may be placed in 
					<< v_model->m_dwCoverage		//Coverage
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< v_model->m_wHooks;			//Hook Type (WORD, no padding)
					break;
					}	
					case 0x00000800:		//Gems
					{
				cmApproachVendor 
					<< WORD(0xFFFF)
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2;	//Object flags 2
					if(v_model->m_dwFlags2 & 0x00000008)
						cmApproachVendor << v_model->m_dwValue;
					if(v_model->m_dwFlags2 & 0x00000010)
						cmApproachVendor << v_model->m_dwUnknown_v2;		
					if(v_model->m_dwFlags2 & 0x00000080)
						cmApproachVendor << v_model->m_dwIconHighlight;
					if(v_model->m_dwFlags2 & 0x00001000)
						cmApproachVendor << v_model->m_wStack;
					if(v_model->m_dwFlags2 & 0x00002000)
						cmApproachVendor << v_model->m_wStackLimit;
					if(v_model->m_dwFlags2 & 0x00004000)
						cmApproachVendor << m_dwGUID;
					if(v_model->m_dwFlags2 & 0x00200000)
						cmApproachVendor << v_model->m_wBurden;
					if(v_model->m_dwFlags2 & 0x00400000)
						cmApproachVendor << v_model->m_wSpellID;
					if(v_model->m_dwFlags2 & 0x01000000)
						cmApproachVendor << v_model->m_fWorkmanship;
					if(v_model->m_dwFlags2 & 0x80000000)
						cmApproachVendor << v_model->m_dwMaterialType;
					cmApproachVendor << WORD(0x0000);
					break;
					}	
					case 0x00001000:		//Spell Components
					{
				cmApproachVendor 
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< WORD(0xFFFF)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//00207019
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< v_model->m_wStack			//NUmber of items in stack 
					<< v_model->m_wStackLimit		//Stack limit
					<< m_dwGUID						//Container ID
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< WORD(0x0000);
		
		//		if _DEBUG
		//		char* debug = ("%s Value is: %d",v_model->m_strName.c_str(),v_model->m_dwValue);
		//		cMasterServer::WriteToFile(debug);
					break;
					}
					case 0x00002000:		//Books,Parchment,scrolls
					{
				cmApproachVendor 
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< WORD(0xFFFF)					
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< m_dwGUID						//Container ID
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< v_model->m_wSpellID;			//SpellID
					break;
					}
					case 0x00004000:		//Lockpicks/Keys
					{
				cmApproachVendor 
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< WORD(0xFFFF)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2;	//Object flags 2
					if(v_model->m_dwFlags2 & 0x00000008)
						cmApproachVendor << v_model->m_dwValue;
					if(v_model->m_dwFlags2 & 0x00000010)
						cmApproachVendor << DWORD(0x00200008);
					if(v_model->m_dwFlags2 & 0x00080000)
						cmApproachVendor << v_model->m_dwUseableOn;
					if(v_model->m_dwFlags2 & 0x00000400)
						cmApproachVendor << v_model->m_wUses;
					if(v_model->m_dwFlags2 & 0x00000800)
						cmApproachVendor << v_model->m_wUseLimit;
					if(v_model->m_dwFlags2 & 0x00004000)
						cmApproachVendor << m_dwGUID;
					if(v_model->m_dwFlags2 & 0x00200000)
						cmApproachVendor << v_model->m_wBurden
											<< WORD(0x0000);
					break;
					}
					case 0x00008000:		//Casting Items
					{
				cmApproachVendor 
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< WORD(0xFFFF)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< DWORD(0x10)					//No clue what this is, always 10 apparently.
					<< m_dwGUID						//Container ID
					<< v_model->m_dwEquipPossible	//Coverage
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< v_model->m_wHooks;			//Hook Type (WORD, no padding)
					break;
					}
					case 0x00040000:		//Trade Notes*
					{
				cmApproachVendor 
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< WORD(0xFFFF)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//00207019
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< v_model->m_wStack			//NUmber of items in stack 
					<< v_model->m_wStackLimit		//Stack limit
					<< m_dwGUID						//Container ID
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< WORD(0x0000);
					break;
					}
					case 0x00080000:		//Manastones*
					{
				cmApproachVendor 
					<< WORD(0xFFFF)					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< WORD(0xFFFF)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2;		//Unknown_10

					if(v_model->m_dwFlags2 & 0x00000080)
					{
						cmApproachVendor <<	v_model->m_dwIconHighlight;
					}
				cmApproachVendor
					<< DWORD(0x891F)				//Usableon
					<< m_dwGUID						//Container ID
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< v_model->m_wHooks;			//Hook Type (WORD, no padding)
					break;
					}

					case 0x00100000:		//Services*
					{
				cmApproachVendor 
					<< Inventory					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//00404018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< m_dwGUID						//Container ID
					<< v_model->m_dwSpellID;		//Spell ID
					break;
					}
					case 0x00400000:		//Cooking Ingredients
					{
				cmApproachVendor 
					<< Inventory					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< m_dwGUID						//Container ID
					<< v_model->m_dwEquipPossible	//EquipMask :the potential equipment slots this object may be placed in 
					<< v_model->m_dwCoverage		//Coverage
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< v_model->m_wHooks;			//Hook Type (WORD, no padding)
					break;
					}
					case 0x08000000:		//Loose fletching supplies
					{
				cmApproachVendor 
					<< Inventory					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2;	//Object flags 2
						if(v_model->m_dwFlags2 & 0x00000008)
						{
							cmApproachVendor
								<< v_model->m_dwValue;	
						}
						if(v_model->m_dwFlags2 & 0x00000010)
						{
							cmApproachVendor
								<< 0x00080008;
						}
						if(v_model->m_dwFlags2 & 0x00080000)
						{
							cmApproachVendor
								<< 0x08000000;
						}
						if(v_model->m_dwFlags2 & 0x00001000)
						{
							cmApproachVendor
								<< v_model->m_wStack;
						}
						if(v_model->m_dwFlags2 & 0x00002000)
						{
							cmApproachVendor
								<< v_model->m_wStackLimit;
						}
						if(v_model->m_dwFlags2 & 0x00004000)
						{
							cmApproachVendor
								<< m_dwGUID;
						}
						if(v_model->m_dwFlags2 & 0x00200000)
						{
							cmApproachVendor	
								<< v_model->m_wBurden;
						}

					break;
					}
					case 0x04000000:		//Alchemy items
					{
				cmApproachVendor 
					<< Inventory					//How many of the item the merchant has (-1 or FFFFFFFF should be unlimited)
					<< v_item->GetGUID()			//Item GUID
					<< v_model->m_dwFlags2			//10254018
					<< v_model->m_strName.c_str()	//Item Name
					<< v_model->m_wModel			//Model
					<< v_model->m_wIcon				//Icon
					<< v_model->m_dwObjectFlags1	//Object Flags 1 (determines sell cats for vendor)
					<< v_model->m_dwObjectFlags2	//Object flags 2
					<< v_model->m_dwValue			//Value
					<< v_model->m_dwUnknown_v2		//Unknown_10
					<< m_dwGUID						//Container ID
					<< v_model->m_dwEquipPossible	//EquipMask :the potential equipment slots this object may be placed in 
					<< v_model->m_dwCoverage		//Coverage
					<< v_model->m_wBurden			//Burden (WORD, no padding)
					<< v_model->m_wHooks;			//Hook Type (WORD, no padding)
					break;
					}
				}	
			}
		
		who->AddPacket(WORLD_SERVER,cmApproachVendor,4);

		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
		who->AddPacket(WORLD_SERVER,cmActionComplete,4);
		}
	}
}

/**
 *	Handles the giving of items to NPCs.
 *
 *	This function is called whenever a player gives an NPC an item.
 *	If the NPC is a Town Crier, the NPC should access the item.
 *	Other NPCs should only accept particular items or types of items.
 *
 *	@param *who - A pointer to the client whose avatar is giving the item.
 *	@param itemid - The GUID of the item being given.
 */
void cNPC::GiveItem(cClient *who, DWORD npcid, DWORD itemid)
{
	//Find the appropriate response to receiving the item in the database.
	char szCommand[100];
	RETCODE retcode;
	SQLCHAR FinishText[512];
	DWORD quest_id;
	DWORD reward_xp;

	sprintf( szCommand, "SELECT * FROM quests WHERE item_id = %d;",this->m_qitem_id1 );

	cMasterServer::ServerMessage(ColorBrown,who,"npc id is: %d, quest item id is: %d",npcid,this->m_qitem_id1);

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	int iCol = 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol, SQL_C_ULONG, &quest_id, sizeof( quest_id ), NULL );  CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)		
	iCol += 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol, SQL_C_ULONG, &reward_xp, sizeof( reward_xp ), NULL );  CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)				
	iCol += 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol, SQL_C_CHAR, FinishText, sizeof( FinishText ), NULL ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	//If we get here, things are good....

	//Find the item in the avatars inventory
	cObject *pcObj2 = who->m_pcAvatar->FindInventory(itemid);

	//Send the appropriate give message to server
	cMessage cmGiveItem;
	cmGiveItem << 0xF7B0L << who->m_pcAvatar->GetGUID() << ++who->m_pcAvatar->m_dwF7B0Sequence << 0x0022L
	<< itemid << npcid << 0L << 0L;
	who->AddPacket(WORLD_SERVER,cmGiveItem,4);

	if( SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS )
	{
		//Convert the data
		std::string ft;
		char *szFT = reinterpret_cast<char *>(FinishText);
		ft.assign(szFT);
		retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		//Send the appropriate quest finish message
		cMasterServer::ServerMessage(ColorBrown,who,"%s tells you, \"%s\"",m_strName.c_str(),ft.c_str());
		//Give any experience rewards
		cMasterServer::ServerMessage(ColorGreen,who,"You've recieved %d experience.",reward_xp);

		who->AddPacket( WORLD_SERVER, who->m_pcAvatar->UpdateUnassignedExp(reward_xp), 4 );
		who->m_pcAvatar->UpdateUnassignedXPDB(who,reward_xp);

		who->AddPacket( WORLD_SERVER, who->m_pcAvatar->UpdateTotalExp(reward_xp), 4);		
		who->m_pcAvatar->UpdateTotalXPDB(who,reward_xp);

		who->AddPacket( WORLD_SERVER, who->m_pcAvatar->UpdateLevel(), 4);
		who->m_pcAvatar->UpdateLevelDB(who);

		//retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

		who->m_pcAvatar->UpdateQuestCompletedTable(who->m_pcAvatar->GetGUID(),quest_id);
	}
	else
	{
		//clean up
		
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
	}

	//Remove the item
	who->m_pcAvatar->DeleteFromInventory(pcObj2);
	cMessage cmRemoveItem;
	cmRemoveItem << 0x0024L << pcObj2->GetGUID();
	who->AddPacket(WORLD_SERVER,cmRemoveItem,4);
	cWorldManager::RemoveObject(pcObj2,true,true);

	//Done
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_pcAvatar->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket( WORLD_SERVER,cmActionComplete,4);
}