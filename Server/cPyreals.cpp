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
 *	@file cPyreals.cpp
 *	Implements functionality for pyreals.
 *
 *	This class is referenced whenever a pyreal object is created, used, stacked, split, or assessed.
 *	Inherits from the cObject class.
 */

#include "avatar.h"
#include "object.h"
#include "cItemModels.h"
#include "client.h"
#include "WorldManager.h"
#include "MasterServer.h"

/**
 *	Handles the message sent for the creation of pyreals in the world.
 *
 *	This function is called whenever a pyreal (or group/stack of pyreals) should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cPyreals::CreatePacket( )
{
	cMessage cmReturn;

	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	if( pcModel )
	{
		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
		cmReturn	<< pcModel->m_bPaletteChange
					<< pcModel->m_bTextureChange
					<< pcModel->m_bModelChange;

		// The Model Vectors
		if ( pcModel->m_bPaletteChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bPaletteChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorPal[i],sizeof(pcModel->m_vectorPal[i]));
			}
		}
		
		if (pcModel->m_bPaletteChange != 0) 
		{
			//Cubem0j0:  Test code for armor only.
			cmReturn << WORD( 0x0C50 );
		}
		
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
	}

	cmReturn.pasteAlign(4);
	cmReturn << pcModel->m_dwFlags1 << 0x414L << 0x65L;

	// MASK 0x00008000 -- Location 
	if ( !m_fIsOwned )
	cmReturn.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );
/*
	// MASK 0x00000800 -- Sound Set
	if (pcModel->m_dwFlags1 & 0x00000800 == 0x00000800) { 
		DWORD dwSoundSet = 0x20000000L + pcModel->m_wSoundSet;
		cmReturn << dwSoundSet;
	}

	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	if (pcModel->m_dwFlags1 & 0x00001000 == 0x00001000) { 
		cmReturn << 0x34000000 + pcModel->m_dwUnknown_Blue;
	}
*/
	// MASK 0x00000001 -- ModelNumber
	DWORD dwModel = 0x02000000L + pcModel->m_dwModelNumber;
	cmReturn << dwModel;

	// SeaGreens
	WORD wNuminteracts = 0x0;
	WORD wNumbubbles = 0x0;
	WORD wNumJumps = 0x0;
	WORD wNumOverrides = 0x0;
	WORD wUnkFlag8 = 0x0;
	WORD wUnkFlag10 = 0x0;
	
	cmReturn	<< m_wPositionSequence
				<< m_wNumAnims //wNuminteracts 
				<< wNumbubbles 
				<< wNumJumps 
				<< m_wNumPortals 
				<< m_wNumAnims 
				<< wNumOverrides 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;

	cmReturn << pcModel->m_dwFlags2;
	cmReturn << Name( );					// Objects Name
	cmReturn << pcModel->m_wModel;			// Objects Model
	cmReturn << pcModel->m_wIcon;			// Objects Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

	if(pcModel->m_dwFlags2 & 0x00000008)
		cmReturn << this->m_wStack;
	if(pcModel->m_dwFlags2 & 0x00000010)
		cmReturn << pcModel->m_dwUnknown_v2;
	if(pcModel->m_dwFlags2 & 0x00001000)
		cmReturn << this->m_wStack;
	if(pcModel->m_dwFlags2 & 0x00002000)
		cmReturn << this->m_wStackLimit;

	return cmReturn;
}

/**
 *	Handles the message sent for the creation of pyreals in a container.
 *
 *	This function is called whenever a pyreal (or group/stack of pyreals) should be created in the inventory of another object.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cPyreals::CreatePacketContainer(DWORD Container, DWORD ItemModelID)
{
	cMessage cmReturn;

	cItemModels *pcModel = cItemModels::FindModel( ItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	if( pcModel )
	{
		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
		cmReturn	<< pcModel->m_bPaletteChange
					<< pcModel->m_bTextureChange
					<< pcModel->m_bModelChange;

		// The Model Vectors
		if ( pcModel->m_bPaletteChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bPaletteChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorPal[i],sizeof(pcModel->m_vectorPal[i]));
			}
		}
		
		if (pcModel->m_bPaletteChange != 0) 
		{
			//Cubem0j0:  Test code for armor only.
			cmReturn << WORD( 0x0C50 );
		}
		
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
	}

	cmReturn.pasteAlign(4);
	cmReturn << pcModel->m_dwFlags1 << 0x414L << 0x65L;

	// MASK 0x00000001 -- ModelNumber
	DWORD dwModel = 0x02000000L + pcModel->m_dwModelNumber;
	cmReturn << dwModel;

	// SeaGreens
	WORD wNuminteracts = 0x0;
	WORD wNumbubbles = 0x0;
	WORD wNumJumps = 0x0;
	WORD wNumOverrides = 0x0;
	WORD wUnkFlag8 = 0x0;
	WORD wUnkFlag10 = 0x0;
	
	cmReturn	<< m_wPositionSequence
				<< m_wNumAnims //wNuminteracts 
				<< wNumbubbles 
				<< wNumJumps 
				<< m_wNumPortals 
				<< m_wNumAnims 
				<< wNumOverrides 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;

	//DWORD dwFlags2 = 0x00007018;

	cmReturn << pcModel->m_dwFlags2;
	cmReturn << Name( );					// Objects Name
	cmReturn << pcModel->m_wModel;			// Objects Model
	cmReturn << pcModel->m_wIcon;			// Objects Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

	DWORD Value = DWORD(this->m_wStack);

	if(pcModel->m_dwFlags2 & 0x00000008)
		cmReturn << Value;
	if(pcModel->m_dwFlags2 & 0x00000010)
		cmReturn << pcModel->m_dwUnknown_v2;
	if(pcModel->m_dwFlags2 & 0x00001000)
		cmReturn << this->m_wStack;
	if(pcModel->m_dwFlags2 & 0x00002000)
		cmReturn << this->m_wStackLimit;
	if(pcModel->m_dwFlags2 & 0x00004000)
		cmReturn << Container;

	return cmReturn;
}

/**
 *	Handles the actions of pyreal objects.
 *
 *	This function is called whenever a pyreal is used or should perform an action.
 */
void cPyreals::Action(cClient *who)
{

}

/**
 *	Handles the stacking of pyreal objects.
 *
 *	This function is called whenever pyreals are stacked (one pyreal or group of pyreals added or stacked to another).
 */
void cPyreals::Stack(cClient *who, DWORD item1, DWORD item2, DWORD amount)
{

}

/**
 *	Handles the splitting of pyreal objects.
 *
 *	This function is called whenever pyreals are split (one pyreal or group of pyreals removed from or split with its group).
 */
void cPyreals::Split(cClient *who, DWORD item_guid, DWORD slot, DWORD value)
{
	
	cObject *Original = who->m_pcAvatar->FindInventory(item_guid);
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );

	DWORD org_stack = DWORD(this->m_wStack);
	DWORD org_value = this->m_dwValue - value;

	cMasterServer::ServerMessage(ColorBlue,NULL,"Orig Stack: %ul",org_stack);

	//k109: Minus the value from original item
	DWORD newstack = org_stack - value;

	
	//k109: Send the 0x0197 message
	cMessage AdjustStack;
	AdjustStack << 0x0197L << BYTE(0x01) << item_guid << this->m_dwValue - value << org_stack;
	who->AddPacket(WORLD_SERVER,AdjustStack,4);

	cMasterServer::ServerMessage(ColorBlue,NULL,"Adjust Stack Fired");
	//k109:  Create the new item, its stack set to value
	cPyreals* cash = new cPyreals(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),m_dwItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,value,WORD(value),pcModel->m_wStackLimit);
	who->AddPacket( WORLD_SERVER, cash->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(cash);
	cMasterServer::ServerMessage(ColorBlue,NULL,"Pyreals Created");

	//k109:  Add the newly created item to the slot in the message
	cMessage cmInsInv;
	cmInsInv << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << cash->GetGUID() << who->m_pcAvatar->GetGUID( ) << slot << 0L;
	who->AddPacket( WORLD_SERVER, cmInsInv, 4 );

	cMasterServer::ServerMessage(ColorBlue,NULL,"Insert Inventory");

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the assessment of jewelry objects.
 *
 *	This function is called whenever jewelry is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cPyreals::Assess( cClient *pcAssesser )
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );

	cMessage cmAssess;
	DWORD flags = 0x0000001;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags 
	<< 0x01L						//Success = 0x01, Failure = 0x00
	<< WORD(0x0002)
	<< WORD(0x0010)
	<< 0x13L						//Value
	<< DWORD(this->m_wStack)			//Amount
	<< 0x05L						//Burden
	<< 0x00L;

	pcAssesser->AddPacket(WORLD_SERVER, cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}