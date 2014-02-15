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
 *	@file cFood.cpp
 *	Implements functionality for food (apples, bread, etc).
 *
 *	This class is referenced whenever food is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "avatar.h"
#include "object.h"
#include "cItemModels.h"
#include "client.h"
#include "WorldManager.h"
#include "MasterServer.h"

/**
 *	Handles the message sent for the creation of food in the world.
 *
 *	This function is called whenever food should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cFood::CreatePacket()
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
			cmReturn << WORD( pcModel->m_wUnknown1 );
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

	// MASK 0x00000800 -- Sound Set 
	DWORD dwSoundSet = 0x20000000L + pcModel->m_wSoundSet;
	cmReturn << dwSoundSet;

	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	cmReturn << 0x34000000 + pcModel->m_dwUnknown_Blue;

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
	
	cmReturn << pcModel->m_dwFlags2;		// Object Flags
	cmReturn << Name( );					// Object's Name
	cmReturn << pcModel->m_wModel;			// Object's Model
	cmReturn << pcModel->m_wIcon;			// Object's Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

	if(pcModel->m_dwFlags2 & 0x00000008)
	{
		cmReturn << 0x0FL;
	}
	// Mask 0x0010 dwUnknown_v2
	cmReturn << DWORD(0x00000020);
	// Mask 0x1000 Stack size
	cmReturn << WORD(0x0002);
	// Mask 0x2000 Stack Limit
	cmReturn << WORD(0x0064);
	// Mask 0x00200000 - Burden
	cmReturn << WORD(0x0040);

	pcObject->SetType(2);

	return cmReturn;
}

/**
 *	Handles the message sent for the creation of food in a container.
 *
 *	This function is called whenever food should be created in the inventory of another object.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cFood::CreatePacketContainer( DWORD Container, DWORD ItemModelID )
{
	cMessage cmReturn;
	cItemModels *pcModel = cItemModels::FindModel( ItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	if( pcModel )
	{

		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11) //0x11 is a constant
					<< pcModel->m_bPaletteChange
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
			cmReturn << WORD( pcModel->m_wUnknown1 );
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
	
	cmReturn << 0x00021801 << 0x414L << 0x65L;

	// MASK 0x00000800 -- Sound Set 
	DWORD dwSoundSet = 0x20000000L + pcModel->m_wSoundSet;
	cmReturn << dwSoundSet;

	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	cmReturn << 0x34000000 + pcModel->m_dwUnknown_Blue;

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
	cmReturn << Name( );					// Object's Name
	cmReturn << pcModel->m_wModel;			// Object's Model
	cmReturn << pcModel->m_wIcon;			// Object's Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;

	if(pcModel->m_dwFlags2 & 0x00000008)
		cmReturn << 0x0L;
	if(pcModel->m_dwFlags2 & 0x00000010)
		cmReturn << DWORD(0x00000020);
	if(pcModel->m_dwFlags2 & 0x00001000)
		cmReturn << this->m_wStack;
	if(pcModel->m_dwFlags2 & 0x00002000)
		cmReturn << pcModel->m_wStackLimit;
	if(pcModel->m_dwFlags2 & 0x00004000)
		cmReturn << Container;
	if(pcModel->m_dwFlags2 & 0x00200000)
		cmReturn << pcModel->m_wBurden;

	return cmReturn;
}

/**
 *	Handles the actions of food objects.
 *
 *	This function is called whenever food is used or should perform an action.
 */
void cFood::Action(cClient* who)
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );
	//cFood *pcObject = reinterpret_cast <cFood *>(cWorldManager::FindObject( m_dwGUID ));
	//cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	int updated_stats;
	int change = who->m_pcAvatar->GetTotalStamina();
	int ret_amount;

	//Lift to mouth animation  
	cMessage cAnimation1 = who->m_pcAvatar->Animation(26,1.0f);
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cAnimation1, 3 );

	//which food is this (CurVitalID)
	
	switch(this->m_dwVitalID)
	{
		case 0x00000002:
		{
			//Update Health
			cMessage cmEat = who->m_pcAvatar->UpdateHealth(this->m_dwAmount,updated_stats);
			who->AddPacket( WORLD_SERVER,cmEat,4);

			if ( updated_stats > change )
				ret_amount = 0;
			else
				ret_amount = this->m_dwAmount;

			cMasterServer::ServerMessage( ColorGreen,who,"The %s restores %d points of your health.",pcModel->m_strName.c_str(),ret_amount);
		}
		break;

		case 0x00000004:
		{
			//Update Stamina
			cMessage cmEat = who->m_pcAvatar->UpdateStamina(this->m_dwAmount,updated_stats);
			who->AddPacket( WORLD_SERVER,cmEat,4);

			if ( updated_stats > change )
				ret_amount = 0;
			else
				ret_amount = this->m_dwAmount;

			cMasterServer::ServerMessage( ColorGreen,who,"The %s restores %d points of your stamina.",pcModel->m_strName.c_str(),ret_amount);
		}
		break;

		case 0x00000006:
		{
			//Update Mana
			cMessage cmEat = who->m_pcAvatar->UpdateMana(this->m_dwAmount,updated_stats);
			who->AddPacket( WORLD_SERVER,cmEat,4);

			if ( updated_stats > change )
				ret_amount = 0;
			else
				ret_amount = this->m_dwAmount;

			cMasterServer::ServerMessage( ColorGreen,who,"The %s restores %d points of your stamina.",pcModel->m_strName.c_str(),ret_amount);
		}
		break;
	}

	//Put hand back down		
	cMessage cAnimation2 = who->m_pcAvatar->Animation(0,1.0f);
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cAnimation2, 3 );

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket( WORLD_SERVER,cmActionComplete,4);
	
	who->m_pcAvatar->DeleteFromInventory(this);

	cDatabase::RemoveFromInventoryDB(who->m_pcAvatar->GetGUID(),m_dwGUID);
	
	cMessage cmRemoveItem;
	cmRemoveItem << 0x0024L << m_dwGUID;
	who->AddPacket(WORLD_SERVER,cmRemoveItem,4);
	
}

/**
 *	Handles the assessment of food objects.
 *
 *	This function is called whenever food is assessed by a client.
  *
*	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cFood::Assess(cClient *pcAssesser)
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );

	cMessage cmAssess;
	DWORD flags = 0x00000009;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags 
	<< 0x01L					//Success = 0x01, Failure = 0x00
	<< WORD(0x0004)				//Total number of DWORDS
	<< WORD(0x0010)				//Unknown
	<< 0x13L					//Value
	<< this->m_dwValue				// 4 pyreals
	<< 0x05L					//Burden
	<< DWORD (this->m_wBurden)		//50 bu
	<< 0x59L					//Affects (Vital ID)
	<< this->m_dwVitalID			//0x04 - Stamina
	<< 0x5AL					//Affects (Amount of vital restored when used)
	<< this->m_wStack			//Amount

    << WORD(0x0001)				//Count (as above) total number of DWORDS
	<< WORD(0x0008)				//Unknown
	<< 0x0EL					//String
	<< 0x0AL;					//This should be amount of characters, but does not seem to affect anything.

	pcAssesser->AddPacket(WORLD_SERVER, cmAssess,4);	
}