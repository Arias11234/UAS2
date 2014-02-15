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
 *	@file cWorldObject.cpp
 *	World objects are defined as objects with which avatars can interact, but not pick up.
 *	This class is referenced whenever a world object is created, used, or assessed.
 *	Inherits from the cObject class.
 *
 *	Author: Cubem0j0
 */

#include "Object.h"
#include "MasterServer.h"
#include "WorldManager.h"
#include "CommandParser.h"

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of world objects (town signs, wells, etc).
 *
 *	Called whenever a world object should be initialized.
 */
cWorldObject::cWorldObject(WORD type, DWORD dwGUID, cLocation *pcLoc, char *szName, char *szDesc)
{
	SetLocation(pcLoc);
	m_dwGUID = dwGUID;
	m_strName.assign( szName );
	m_strDescription.assign( szDesc );
	m_dwModel = type;
	m_fStatic = TRUE;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of world objects in the world.
 *
 *	This function is called whenever a world object should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cWorldObject::CreatePacket()
{
	DWORD dwFlags;
	cMessage cmCreate;

	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 )
				<< BYTE( 0 )
				<< BYTE( 0 )
				<< BYTE( 0 );

	dwFlags = 0x00028001L;
	WORD wPortalMode = 0x0408;

	cmCreate << dwFlags;
	cmCreate << WORD( wPortalMode );
	cmCreate << WORD( 0x8005 ); // wUnknown_1
	

	// MASK 0x00020000 unknown  
	cmCreate << 0x65L;

	// MASK 0x00008000 - Location 
	if ( !m_fIsOwned )
		cmCreate.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );

	// MASK 0x00000001 ModelNumber
	DWORD dwModel = 0x02000000L + m_dwModel;
	cmCreate << dwModel;

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
	
	switch(iObjectType)
	{
	case 0:
		{
			DWORD dwFlags2 = 0x00200018;
	
			cmCreate << dwFlags2 << m_strName.c_str( ) << WORD(m_wModel) << WORD(m_wIcon);

			DWORD dwObjectFlags1 = 0x80L;	
			cmCreate << dwObjectFlags1;

			DWORD dwObjectFlags2 = 0x80L;
			cmCreate << dwObjectFlags2;
			// Masked against dwFlags2
			//Mask 0x0008 - Value
			cmCreate << DWORD(0x00);
			// Mask 0x0010 dwUnknown_v2
			cmCreate << DWORD(0x20);
			// Mask 0x00200000 - Burden
			cmCreate << WORD(0x0040);
		}
		break;
	case 1:
		{
			DWORD dwFlags2 = 0x00200018;
	
			cmCreate << dwFlags2 << m_strName.c_str( ) << WORD(m_wModel) << WORD(m_wIcon);

			DWORD dwObjectFlags1 = 0x80L;	
			cmCreate << dwObjectFlags1;

			DWORD dwObjectFlags2 = 0x14L;
			cmCreate << dwObjectFlags2;
			// Masked against dwFlags2
			//Mask 0x0008 - Value
			cmCreate << DWORD(0x00);
			// Mask 0x0010 dwUnknown_v2
			cmCreate << DWORD(0x20);
			// Mask 0x00200000 - Burden
			cmCreate << WORD(0x0040);
		}
		break;
	case 2:
		{
			DWORD dwFlags2 = 0x00600038;
	
			cmCreate << dwFlags2 << m_strName.c_str( ) << WORD(m_wModel) << WORD(m_wIcon);

			DWORD dwObjectFlags1 = 0x00400000;	
			cmCreate << dwObjectFlags1;

			DWORD dwObjectFlags2 = 0x14L;
			cmCreate << dwObjectFlags2;

			// Masked against dwFlags2
			//Mask 0x0008 - Value
			cmCreate << DWORD(0x00);
			// Mask 0x0010 dwUnknown_v2
			cmCreate << DWORD(0x20);
			// Mask 0x0020 Approach distance
			cmCreate << float(3.0);
			// Mask 0x00200000 - Burden
			cmCreate << WORD(0x0040);
			// Mask 0x00400000 - Spell
			cmCreate << WORD(0x049F);
		}
		break;
	default:
		{
			DWORD dwFlags2 = 0x00200018;
	
			cmCreate << dwFlags2 << m_strName.c_str( ) << WORD(m_wModel) << WORD(m_wIcon);
			DWORD dwObjectFlags1 = 0x80L;	
			cmCreate << dwObjectFlags1;

			DWORD dwObjectFlags2 = 0x14L;
			cmCreate << dwObjectFlags2;
			// Masked against dwFlags2
			//Mask 0x0008 - Value
			cmCreate << DWORD(0x00);
			// Mask 0x0010 dwUnknown_v2
			cmCreate << DWORD(0x20);
			// Mask 0x00200000 - Burden
			cmCreate << WORD(0x0040);
		}
		break;
	}

	
	return cmCreate;
}	

/**
 *	Handles the actions of world objects.
 *
 *	This function is called whenever a world object is used or should perform an action.
 */
void cWorldObject::Action(cClient *who)
{
	switch(iObjectType)
	{
	case 1:
		{
			break;
		}
	case 2:
		{
			int updated_stats;
			int change = who->m_pcAvatar->GetTotalStamina();
			int ret_amount;
			int iRand = rand();

			iRand = (4 % 8 );

			//Update Stamina
			cMessage cmEat = who->m_pcAvatar->UpdateStamina(iRand,updated_stats);
			who->AddPacket( WORLD_SERVER,cmEat,4);

			if ( updated_stats > change )
				ret_amount = 0;
			else
				ret_amount = iRand;

			cMessage cmCast;
			cmCast << 0xF750L << who->m_pcAvatar->GetGUID() << DWORD(0x80);
			cMessage cParticle = who->m_pcAvatar->Particle(34);
			cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cParticle, 3 );
			
			cMasterServer::ServerMessage(ColorBlue,who,"%s casts Revitalize I on you restoring %d points of Stamina.",this->m_strName.c_str(),ret_amount);
			who->AddPacket(WORLD_SERVER,cmCast,3);

			//Action Complete
			cMessage cmActionComplete;
			cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
			who->AddPacket(WORLD_SERVER,cmActionComplete,4);

			break;
		}
	}

}

/**
 *	Handles the assessment of world objects.
 *
 *	This function is called whenever a world objects is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cWorldObject::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	//Fill out basic header
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L;
	cmAssess << m_dwGUID << 0x8L <<  1L << WORD(0x01) << WORD(0x0) << WORD(0x10) << "This is a life stone.  When you die, you resurrect here.  SO DON'T DIE!!!";
	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);
}