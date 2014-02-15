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
 *	@file cHealingKits.cpp
 *	Implements functionality for healing kits.
 *
 *	This class is referenced whenever a healing kit is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "avatar.h"
#include "object.h"
#include "cItemModels.h"
#include "client.h"
#include "WorldManager.h"
#include "MasterServer.h"
#include "Job.h"

/**
 *	Handles the message sent for the creation of healing kits in the world.
 *
 *	This function is called whenever a healing kit should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cHealingKits::CreatePacket( )
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
	
	if(pcModel->m_dwContainerID != 0)
	{
		DWORD owned = pcModel->m_dwFlags2 & 0x00004000;
		cmReturn << owned;
	}
	else
	{
		cmReturn << pcModel->m_dwFlags2;		// Object Flags
	}

	cmReturn << Name( );					// Object's Name
	cmReturn << pcModel->m_wModel;			// Object's Model
	cmReturn << pcModel->m_wIcon;			// Object's Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

	//Healing Kits flag2 - 00280c18

	// Mask 0x0008 - Value
	if(pcModel->m_dwFlags2 & 0x00000008)
	{
		cmReturn << this->m_dwValue;
	}

	// Mask 0x0010 dwUnknown_v2
	cmReturn << DWORD(0x00220008);
		
	// Mask 0x80000 Usable on
	cmReturn << 0x10L;

	// Mase 0x0400 Uses Remaining
	if(pcModel->m_dwFlags2 & 0x00000400)
	{
		cmReturn << this->m_wUses;
	}

	// Mase 0x0800 Max # of uses
	if(pcModel->m_dwFlags2 & 0x00000800)
	{
		cmReturn << this->m_wUseLimit;
	}
		
	// Mask 0x80000 Usable on
	//cmReturn << 0x32L;
		
	// Mask 0x00200000 - Burden
	if(pcModel->m_dwFlags2 & 0x00200000)
	{
		cmReturn << this->m_wWeight;
	}

	return cmReturn;
}

/**
 *	Handles the message sent for the creation of healing kits in a container.
 *
 *	This function is called whenever a healing kit should be created in the inventory of another object.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
void cHealingKits::Action(cClient *who)
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	//Vars
	int updated_stats;
	int max = who->m_pcAvatar->GetTotalHealth();
	DWORD current = who->m_pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent;
	DWORD skill = who->m_pcAvatar->m_cStats.m_lpcSkills[15].m_wStatus;
	int ret_heal;
	int total;

	//If we are not damaged, do nothing.
	if (current == max)
	{
		//This is the message that tell you that you don't need to be healed.
		cMessage cmTest;
		cmTest << 0xF7B0L << who->m_pcAvatar->GetGUID() << ++who->m_dwF7B0Sequence << 0x028BL << 0x04FFL << who->m_pcAvatar->m_strName.c_str();
		who->AddPacket(WORLD_SERVER,cmTest,4);

	}
	else
	{
		//Healing kit animation
		//note: 17 is death animation

		//cMessage aHealingKit = who->m_pcAvatar->Animation(270,1.0f);
		//cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, aHealingKit, 3 );
		
//		cAvatarGenericAnimation* Anim = new cAvatarGenericAnimation();
		
//		int iJob = cMasterServer::m_pcJobPool->CreateJob( &cAvatar::Animation( 270, 1.0f ), (LPVOID) Anim, NULL, "Healing_Animation", 10, 1);

		//Figure out how much we healed based on skill
		//Healing = 2 * (max health - current health) with 50% base.  Example 200 max, 50 current 2* 150 = 300, base 300 skill to have 50% chance to heal.

		DWORD amt_healed = (max - current) * 2;
		
		//For now just check if we rolled higher then skill
		if (amt_healed > skill)
		{
			//Return random health between 1 and 25
			ret_heal = rand() % 25 + 1;
		}
		else
		{
			//Failed, return 0
			ret_heal = 0;
		}
		
		//Do the health update
		cMessage cmHealKit = who->m_pcAvatar->UpdateHealth(ret_heal,updated_stats);
		who->AddPacket( WORLD_SERVER,cmHealKit,4);

		//Minus one use
		this->m_wUses--;

		if ( updated_stats > max )
			total = 0;
		else
			total = ret_heal;

		cMasterServer::ServerMessage( ColorGreen,who,"You heal yourself for %i Health points.  Your %s has %i uses left.",ret_heal,pcModel->m_strName.c_str(),this->m_wUses);

		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
		who->AddPacket(WORLD_SERVER,cmActionComplete,4);

		if(this->m_wUses == 0)
		{
			//Remove the item
			who->m_pcAvatar->DeleteFromInventory(pcObject);
			cMessage cmRemoveItem;
			cmRemoveItem << 0x0024L << m_dwGUID;
			who->AddPacket(WORLD_SERVER,cmRemoveItem,4);
		}
	}
}

/**
 *	Handles the assessment of healing kit objects.
 *
 *	This function is called whenever a healing kit is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cHealingKits::Assess( cClient *pcAssesser )
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );

	cMessage cmAssess;
	DWORD flags = 0x00000009;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags 
	<< 0x01L
	<< WORD(0x05)
	<< WORD(0x10)
	<< 0x13L
	<< pcModel->m_dwValue
	<< 0x05L
	<< DWORD(pcModel->m_wBurden)
	<< 0x5AL	//Healing kit bonus
	<< 0x02L
	<< 0x5BL
	<< DWORD(this->m_wUseLimit)
	<< 0x5CL
	<< DWORD(this->m_wUses)

/* Cubem0j0:  This part does not work.  The client doesn't seem to know what this is.
	<< WORD(0x01)
	<< WORD(0x08)
	<< 0x64L		//Healing kit bonus
	<< 0x20L
*/
	<< WORD(0x01)
	<< WORD(0x08)
	<< 0x10L
	<< pcModel->m_strDescription.c_str();

	pcAssesser->AddPacket(WORLD_SERVER, cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}