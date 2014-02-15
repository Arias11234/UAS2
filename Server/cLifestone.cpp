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
 *	@file cLifestone.cpp
 *	Implements functionality for lifestones.
 *
 *	This class is referenced whenever a lifestone is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "Object.h"
#include "MasterServer.h"
#include "WorldManager.h"
#include "CommandParser.h"

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of lifestones.
 *
 *	Called whenever a lifestone object should be initialized.
 */
cLifestone::cLifestone(WORD type, DWORD dwGUID, cLocation *pcLoc, char *szName, char *szDesc)
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
 *	Handles the message sent for the creation of lifestones in the world.
 *
 *	This function is called whenever a lifestone should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cLifestone::CreatePacket()
{
	cMessage cmCreate;
	unsigned char bUnknown[] = {
		0x00, 0x00, // Animation type 0 - 1
		0x3D, 0x00, // Starting Stance 2 - 3
		0x00, 0xF0, 0xAA, 0x02, // Unknown - Flag 0x00000002 in it 4 - 7

	};

	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 );
	
	DWORD dwFlags = 0x00018803L;
	WORD  wPortalMode = 0x0410;

	cmCreate << dwFlags;
	cmCreate << WORD( wPortalMode );
	cmCreate << WORD( 0x0000 ); // wUnknown_1

	// MASK 0x00010000 unknown Bytes - Starting Animation  
	cmCreate << 0x08L;
	cmCreate.pasteData(bUnknown,sizeof(bUnknown));
	cmCreate << 0x0L;

	// MASK 0x00008000 - Location 
	if ( !m_fIsOwned )
		cmCreate.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );
	
	// MASK 0x00000002 Animation Set
	DWORD dwAnimC = 0x09000000L + m_wAnimConfig;
	cmCreate << dwAnimC;

	// MASK 0x00000800 Sound Set 
	DWORD dwSoundSet = 0x20000000L + m_wSoundSet;
	cmCreate << dwSoundSet;

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

	DWORD dwFlags2 = 0x00800030;
	
	cmCreate << dwFlags2 << m_strName.c_str( ) << WORD(m_wModel) << WORD(m_wIcon);
				
	//DWORD dwObjectFlags1 = 0x80L;	
	cmCreate << m_dwObjectsFlag1;

	//DWORD dwObjectFlags2 = 0x1014L;
	cmCreate << m_dwObjectsFlag2 ;

	// Masked against dwFlags2
	// Mask 0x0010 dwUnknown_v2
	cmCreate << DWORD(0x20);
	// Mask 0x0020 - unknown_v3
	cmCreate << float(2.0);
	// Mask 0x00800000 - unknown_v6
	cmCreate << BYTE(0x04);
	
	return cmCreate;
}
	
/**
 *	Handles the actions of lifestone objects.
 *
 *	This function is called whenever a lifestone is used or should perform an action.
 */
void cLifestone::Action(cClient *who)
{
	cMessage cmActionComplete;

	WORD wAnimate = 0x0057;
	float flSpeed = 1.0f;
	cMessage cAnimate = who->m_pcAvatar->Animation(wAnimate, flSpeed);
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cAnimate, 3 );

	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);

	DWORD dwSound = 0x51L;
	cMessage cSound = who->m_pcAvatar->SoundEffect(dwSound,1.0f);
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cSound, 3 );

	cMasterServer::ServerMessage(7,who,"You have attuned your spirit to this Lifestone. You will resurrect here after you die.");
	cLocation pcLoc = who->m_pcAvatar->m_Location;
	DWORD dwGUID = who->m_pcAvatar->GetGUID();
	std::string strName = who->m_pcAvatar->Name();
	cCommandParser::RecordLifestone(who->m_pcAvatar->m_Location,dwGUID);
}

/**
 *	Handles the assessment of lifestone objects.
 *
 *	This function is called whenever a lifestone is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cLifestone::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	//Fill out basic header
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L;
	cmAssess << m_dwGUID << 0x8L <<  1L << WORD(0x01) << WORD(0x0) << WORD(0x10) << "Use this item to set your resurrection point.";
	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);
}