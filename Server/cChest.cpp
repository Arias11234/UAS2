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
 *	@file cChest.cpp
 *	Implements functionality for chests (Sturdy Iron Chests, Singularity Troves, etc).
 *
 *	This class is referenced whenever a chest is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "Client.h"
#include "MasterServer.h"
#include "Object.h"
#include "WorldManager.h"

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of chests.
 *
 *	Called whenever a chest object should be initialized.
 */
cChest::cChest( WORD type, DWORD dwGUID, cLocation *pcLoc, char *szName, char *szDesc)
{
	SetLocation( pcLoc );
	m_dwGUID = dwGUID;//cWorldManager::NewGUID_Object( );
	m_strName.assign( szName );
	m_strDescription.assign( szDesc );
	m_dwModel = type;
	m_dwType = 0;
	m_fStatic = TRUE;
	m_dwDoorState = 0x0CL;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of chests in the world.
 *
 *	This function is called whenever a chest should be created for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cChest::CreatePacket()
{
	cMessage cmCreate;
	unsigned char bUnknown[] = {
		0x00, 0x00, // Animation type 0 - 1
		0x3D, 0x00, // Starting Stance 2 - 3
		0x00, 0x00, 0x00, 0x02, // Unknown - Flag 0x00000002 in it 4 - 7
		0x0C, 0x00, 0x00, 0x00, // Status 9 - 11
	};

	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 );
	
	DWORD dwFlags = 0x00019803L;

	cmCreate << dwFlags;
	cmCreate << WORD( 0x0408 );
	cmCreate << WORD( 0x000C );

	// MASK 0x00010000 unknown Bytes - Starting Animation  
	cmCreate << 0x0CL;

	CopyMemory(&bUnknown[9],&m_dwDoorState,4);
	cmCreate.pasteData(bUnknown,sizeof(bUnknown));
	//cmCreate << 0x003D0000 << 0x02B4F002 << m_dwDoorState; //0x0000000C; // unknownByte
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
	cmCreate << 0x3400002BL;//0x00000000L;

	// MASK 0x00000001 -- ModelNumber
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

	DWORD dwFlags2 = 0x0020003E;
	
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
	DWORD dwObjectFlags2;

	if (GetIsLocked() == 1)
	{
		dwObjectFlags2 = 0x0014L;
	}
	else
	{
		dwObjectFlags2 = 0x0015L;
	}
	
	cmCreate << dwObjectFlags2;

	/* Masked against dwFlags2 in reverse order:
		0x00000002 - Item Slots
		0x00000004 - Pack Slots
		0x00000008 - Value
		0x00000010 - Unknown1
		0x00000020 - Approach Distance
		0x00200000 - Burden
	*/
	cmCreate << BYTE (0x78);		//78 is item slots. (120)
	cmCreate << BYTE (0x06);		//06 is pack slots
	cmCreate << DWORD (0x000009C4); //Value
	cmCreate << DWORD (0x00000030); //Unknown
	cmCreate << float(3.0);			//Approach Distance.
	cmCreate << WORD (0x0225);		//Burden
	
	return cmCreate;
}

/**
 *	Handles the actions of chest objects.
 *
 *	This function is called whenever a chest is used or should perform an action.
 */
void cChest::Action(cClient* who)
{
	cMessage cmUseChest,cmRet;
	WORD m_wStance;

	if(m_wState == 0)  //CLOSED
	{
		m_dwDoorState = 0x0CL;
		m_wStance = 0x3D; // Starting Stance
		m_wState = 1;

	}
	else //OPEN
	{
		m_dwDoorState = 0x0BL;
		m_wStance = 0x3D;
		m_wState = 0;
	}

	//Play Animation
	cmUseChest	<< 0xF74CL					// Packet type
				<< m_dwGUID					// Door's GUID
				<< m_wNumLogins				// number of logins
				<< ++m_wPositionSequence	// sequence number
				<< ++m_wNumAnims			// Number Animations this login session
				<< WORD(0x00)				// Activity
				<< WORD(0x00)
				<< WORD(0x3D)				// Starting Stance
				<< DWORD(0x00000002)		// Supposed to be Animation Mask
				<< m_dwDoorState;			// Door state Closed = 0x0C Open = 0x0B
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cmUseChest, 3 );

	if(GetIsLocked() == 1)
	{
		cMessage cmLocked;
		cmLocked << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x02EBL
			<< "%s is locked!",this->m_strName.c_str();
		who->AddPacket(WORLD_SERVER,cmLocked,4);

		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
		who->AddPacket(WORLD_SERVER,cmActionComplete,4);
	}

	if(m_wState == 0)
	{
		//Display the item slots in the chest (0x0196)
		cMessage cmSetPackContents;
		cmSetPackContents << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x0196L << m_dwGUID;
		who->AddPacket(WORLD_SERVER,cmSetPackContents,4);
		
		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
		who->AddPacket(WORLD_SERVER,cmActionComplete,4);
	}

	if(m_wState == 1)
	{
		//Close Container
		cMessage cmCloseContainer;
		cmCloseContainer << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x0052L << m_dwGUID;
		who->AddPacket(WORLD_SERVER,cmCloseContainer,4);

		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
		who->AddPacket(WORLD_SERVER,cmActionComplete,4);
	}
}

/**
 *	Handles the assessment of chest objects.
 *
 *	This function is called whenever a chest is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cChest::Assess(cClient *pcAssesser)
{
	//Cubem0j0: Note - The pack slots and item slots are added automatically to this assess message.
	cMessage cmAssess;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L;
	cmAssess << m_dwGUID << 0x00400003L << 1L;
	if(this->GetIsLocked())
	{
		cmAssess << WORD(0x03) << WORD(0x10) << 0x13L << 0x94L << 0x05L << 0x96L << 
			WORD(0x01) << WORD(0x08) << 0x03L << 0x01;
	}
	else
	{
		cmAssess << WORD(0x02) << WORD(0x10) << 0x13L << 0x94L << 0x05L << 0x96L;
	}
	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);
}