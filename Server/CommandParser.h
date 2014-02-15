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
 *	@file CommandParser.h
 */

#ifndef __COMMANDPARSER_H
#define __COMMANDPARSER_H

#include "MasterServer.h"
#include "VersionNo.h"

class cClient;

class cCommandParser
{
	friend class cMasterServer;
	friend class cLifestone;

public:
	cCommandParser	( ) {}
	~cCommandParser	( ) {}

private:
	static BOOL Help			( char *szText, BYTE bAccessLevel );
	static BOOL Turn			( char *szHeading );
	static BOOL Telemap			( char *szLocation );
	static BOOL TeleLoc			( char *szLocation );
	static BOOL TeleTown		( char *szTown );
	
	static BOOL SpawnItem		( char *szItem );
	static BOOL SpawnItemLB		( char *szItem );
	
	static BOOL Munster			( );
	static BOOL Dungeon			( char *szDungeon );
	static BOOL Particle		( char *szParticle );
	static BOOL SoundEffect		( char *szSound );
	static BOOL Animation		( char *szAnimation );
	static BOOL Punch			( );
	static BOOL DungeonList		( char *szDungeonGroup );
	static BOOL RemoveInventory	( );
	static BOOL SpawnMonster	( char *szText );
	static BOOL SpawnSave		( char *szText );
	static BOOL Spawntype		( char *szText );
	static BOOL SpawnPlayer		( );
	static BOOL SpawnPortal		( );
	static BOOL RecordLocation	( );
	static BOOL RecordLifestone	( cLocation pcLoc, DWORD dwGUID );
	static BOOL SpwnID			( char *szText );
	static BOOL SpawnID			( char *szText );
	static BOOL GlobalChat		( char *szText );
	static BOOL ClearObjects	( );
	static BOOL WorldBroadcast	( char *szText );
	static BOOL GotoCharacter	( char *szText );
	static BOOL GetCharacter	( char *szText );
	static BOOL ReturnCharacter	( char *szText );
	static BOOL SendCharacter	( char *szText );
	static BOOL Home			();
	static BOOL Invisible		();
	static BOOL Visible			();
	static BOOL Who				();

	static BOOL SetModel		( char *szText );
	static BOOL SetScale		( char *szText );
	static BOOL Acid			( char *szText );

//	static BOOL SaveNPC			( char *szText );
	static BOOL Version			();
	static BOOL Wear			();
	static BOOL Remove			();
	static BOOL ODOA			();
	
	static BOOL RandomPyreals	();

	static inline BOOL FormatError( char *szCommand )
	{
		cMasterServer::ServerMessage( ColorRed, m_pcClient, "Error in %s syntax, check !help", szCommand );
		return FALSE;
	}

	static cClient *m_pcClient;

};
static byte text[64] = { 0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,
		0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,
		0x4D,0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x20,0x2E};

#endif	// #ifndef __COMMANDPARSER_H