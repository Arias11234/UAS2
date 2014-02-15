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
 *	@file cAltar.cpp
 *	Handles the altar object functions.
 *
 *	This class is referenced whenever an altar is created, used, or assessed.
 *	Inherits from the cObject class.
 */

//Cubem0j0:  Need to review, this may be a better fit under World objects.

#include "Client.h"
#include "MasterServer.h"
#include "Object.h"
#include "WorldManager.h"
    
/***************
 *	constructors
 **************/

/**
 *	Handles the creation of altars.
 *
 *	Called whenever an altar object should be initialized.
 */
cAltar::cAltar( DWORD dwType, DWORD dwGUID, cLocation *pcLoc, char* strName, char* strDesc)
{
	SetLocation( pcLoc );
	m_dwGUID		=	dwGUID;
	m_strName.assign( strName );
	m_strDescription.assign( strDesc );
	m_dwType		=	dwType;
	m_dwModel		=	(dwType ? 0x0359 : 0x034E);
	m_wAnimConfig	=	(dwType ? 0x002E : 0x002D);
	m_wSoundSet		=	(dwType ? 0x0034 : 0x0035);
	m_wModel		=	(dwType ? 0x0356 : 0x0357);
	m_wIcon			=	(dwType ? 0x134F : 0x134F);
	m_fStatic		=	TRUE;
	m_wNumPortals	=	0x1;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of altars.
 *
 *	This function is called whenever an altar should be created for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cAltar::CreatePacket()
{
	cMessage cmCreate;
	unsigned char bUnknown[] = {
		0x00, 0x00, // Animation type 0 - 1
		0x3D, 0x00, // Starting Stance 2 - 3
		0x00, 0xF0, 0x7A, 0x01, // Unknown - Flag 0x00000002 in it 4 - 7
	};

	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 );
	
	DWORD dwFlags = 0x00018803L;

	cmCreate << dwFlags;
	cmCreate << WORD( 0x0410 );
	cmCreate << WORD( 0x0000 );

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
				<< m_wNumAnims 
				<< wNumbubbles 
				<< wNumJumps 
				<< m_wNumPortals 
				<< m_wNumAnims 
				<< wNumOverrides 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;

	DWORD dwFlags2 = 0x00000030;
	
	cmCreate	<< dwFlags2 << m_strName.c_str( ) << WORD(m_wModel) << WORD(m_wIcon);

	DWORD dwObjectFlags1 = 0x80L;	
	cmCreate << dwObjectFlags1;

	DWORD dwObjectFlags2 = (m_dwType ? 0x0414 : 0x0814);
	cmCreate << dwObjectFlags2 ;

	// Masked against dwFlags2
	// Mask 0x0010 dwUnknown_v2
	cmCreate << DWORD(0x20);
	// Mask 0x0020 - unknown_v3
	cmCreate << float(2.0);
	
	return cmCreate;
}

/**
 *	Handles the actions of altar objects.
 *
 *	This function is called whenever an altar is used or should perform an action.
 */
void cAltar::Action(cClient* who)
{
	who->m_pcAvatar->m_fIsPK = m_dwType;
	who->m_pcAvatar->SetIsPK_DB();
	DWORD dwFlagToSet;

	switch(m_dwType)
	{
	case 0:
		{
			dwFlagToSet = 0x2L;
			break;
		}
	case 1:
		{
			dwFlagToSet = 0x4L;
			break;
		}
	}
	cMessage cmSetFlag;
	cmSetFlag << 0x022CL << BYTE(0) << who->m_pcAvatar->GetGUID() << 0x4L << m_dwType; // byte(0) pkFlag 0x4L

	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cmSetFlag, 4 );
	cMasterServer::ServerMessage(ColorGreen,who,(m_dwType ? "You hear distant laughter of delight and welcome, as you join the ranks of those freed from Asheron's protective shackles by Bael'Zharon.  You have become one of his Chosen, a Player Killer." : " You are enveloped in a feeling of warmth as you are brought back protection of Light."));
	cMessage cmActionComplete;

	// Particle Effect
	DWORD dwParticle = (m_dwType ? 71 : 141 );
	cMessage cPart = who->m_pcAvatar->Particle( dwParticle );
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cPart, 3 );
	dwParticle = (m_dwType ? 142 : 147 );
	cMessage cPart2 = who->m_pcAvatar->Particle( dwParticle );
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cPart2, 3 );
	// End Particle Effect
	cMessage cmCoverME;
	cmCoverME << 0x0229L << who->m_pcAvatar->m_bFlagCount++ << who->m_pcAvatar->GetGUID() << DWORD(134) << dwFlagToSet;
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cmCoverME, 4 );

	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

//TODO:  Fix this assess message
/**
 *	Handles the assessment of altar objects.
 *
 *	This function is called whenever an altar is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cAltar::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L;
	cmAssess << m_dwGUID << 0x0008L << 1L << 0x0010L << (dwType ? "Use this altar to become PK" : "Use this altar to become an NPK");
	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}