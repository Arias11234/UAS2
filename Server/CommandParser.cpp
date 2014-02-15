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
 *	@file CommandParser.cpp
 *	Implements functionality for command parsing.
 *
 *	Used to parse server-specific commands outside the typical protocol.
 */

#include "Client.h"
#include "CommandParser.h"
#include "WorldManager.h"
#include "DataBase.h"
#include "Wand.h"
#include "Job.h"
#include "TreasureGen.h"

DWORD SpawnLimit = 99999999; // 3400 for non developers

cClient *cCommandParser::m_pcClient;
TreasureGen *tgen = new TreasureGen();

/**
 *	Records an avatar's location to the database.
 */
BOOL cCommandParser::RecordLocation( ) 
{
	#ifdef _DEBUG
		UpdateConsole( " %s's location being recorded ...\r\n", m_pcClient->m_pcAvatar->m_strName.c_str() );
	#endif // _DEBUG

//  if ( !strlen( szCommand ) )
//		return FormatError( "!recordloc" );
		
	DWORD dwLandBlock	= m_pcClient->m_pcAvatar->m_Location.m_dwLandBlock;
//	float flX			= m_pcClient->m_pcAvatar->m_Location.m_flX;
//	float flY			= m_pcClient->m_pcAvatar->m_Location.m_flY;
//	float flZ			= m_pcClient->m_pcAvatar->m_Location.m_flZ;
//	float flA			= m_pcClient->m_pcAvatar->m_Location.m_flA;
//	float flB			= m_pcClient->m_pcAvatar->m_Location.m_flB;
//	float flC			= m_pcClient->m_pcAvatar->m_Location.m_flC;
//	float flW			= m_pcClient->m_pcAvatar->m_Location.m_flW;
	DWORD flX;
	DWORD flY;
	DWORD flZ;
	DWORD flA;
	DWORD flB;
	DWORD flC;
	DWORD flW;	
	DWORD dwGUID		= m_pcClient->m_pcAvatar->m_dwGUID;
	
	BOOL fVerify = TRUE;
	char Command[500];

	RETCODE retcode;

	// floating point to 32-bit hexadecimal
	flX = cDatabase::Float2Hex(m_pcClient->m_pcAvatar->m_Location.m_flX);
	flY = cDatabase::Float2Hex(m_pcClient->m_pcAvatar->m_Location.m_flY);
	flZ = cDatabase::Float2Hex(m_pcClient->m_pcAvatar->m_Location.m_flZ);
	flA = cDatabase::Float2Hex(m_pcClient->m_pcAvatar->m_Location.m_flA);
	flB = cDatabase::Float2Hex(m_pcClient->m_pcAvatar->m_Location.m_flB);
	flC = cDatabase::Float2Hex(m_pcClient->m_pcAvatar->m_Location.m_flC);
	flW = cDatabase::Float2Hex(m_pcClient->m_pcAvatar->m_Location.m_flW);

	// database code to record start location
	sprintf( Command, "UPDATE avatar_location SET Landblock = ('%08x'), Position_X = ('%08x'),Position_Y = ('%08x'),Position_Z = ('%08x'),Orientation_W = ('%08x'),Orientation_X = ('%08x'),Orientation_Y = ('%08x'),Orientation_Z = ('%08x') WHERE AvatarGUID = (%d);",dwLandBlock, flX, flY, flZ, flA, flB, flC, flW, dwGUID );

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)Command, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
	
	if( retcode == SQL_NO_DATA ){
		sprintf( Command, "INSERT INTO avatar_location (AvatarGUID, Landblock , Position_X, Position_Y, Position_Z, Orientation_W, Orientation_X, Orientation_Y,Orientation_Z) VALUES (%d,'%08x',%f,%f,%f,%f,%f,%f,%f);",dwGUID,dwLandBlock, flX, flY, flZ, flA, flB, flC, flW  );

		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)Command, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
	
	}
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

	char szMessage[100];
	sprintf( szMessage, "      <SQL> Recorded location for %s updated.\r\n",m_pcClient->m_pcAvatar->m_strName.c_str() );
	UpdateConsole((char *)szMessage);

	return TRUE;
}

/**
 *	Records an avatar's lifestone to the database.
 */
BOOL cCommandParser::RecordLifestone( cLocation pcLoc,DWORD dwGUID )
{
	char szMessage[100];
	#ifdef _DEBUG
//		sprintf( szMessage, " %s's lifestone location being updated ...\r\n", strName );
		sprintf( szMessage, " Lifestone location being updated for GUID %d ...\r\n", dwGUID );
		UpdateConsole((char *)szMessage);
	#endif // _DEBUG

	DWORD dwLandBlock	= pcLoc.m_dwLandBlock;
//	float flX			= pcLoc.m_flX;
//	float flY			= pcLoc.m_flY;
//	float flZ			= pcLoc.m_flZ;
//	float flA			= pcLoc.m_flA;
//	float flB			= pcLoc.m_flB;
//	float flC			= pcLoc.m_flC;
//	float flW			= pcLoc.m_flW;
	DWORD flX;
	DWORD flY;
	DWORD flZ;
	DWORD flA;
	DWORD flB;
	DWORD flC;
	DWORD flW;

	// database code to record lifestone tie
	BOOL fVerify = TRUE;
	char Command[500];

	RETCODE retcode;

	// floating point to 32-bit hexadecimal
	flX = cDatabase::Float2Hex(pcLoc.m_flX);
	flY = cDatabase::Float2Hex(pcLoc.m_flY);
	flZ = cDatabase::Float2Hex(pcLoc.m_flZ);
	flA = cDatabase::Float2Hex(pcLoc.m_flA);
	flB = cDatabase::Float2Hex(pcLoc.m_flB);
	flC = cDatabase::Float2Hex(pcLoc.m_flC);
	flW = cDatabase::Float2Hex(pcLoc.m_flW);

	sprintf( Command, "UPDATE avatar_location SET LS_LandBlock = ('%08x'), LS_Position_X = ('%08x'),LS_Position_Y = ('%08x'), LS_Position_Z = ('%08x'), LS_Orientation_W = ('%08x'), LS_Orientation_X = ('%08x'), LS_Orientation_Y = ('%08x'), LS_Orientation_Z = ('%08x') WHERE AvatarGUID = (%d);",dwLandBlock, flX, flY, flZ, flA, flB, flC, flW, dwGUID );
//	sprintf( Command, "UPDATE avatar_location SET LS_LandBlock = ('%08x'), LS_Position_X = (%f), LS_Position_Y = (%f), LS_Position_Z = (%f), LS_Orientation_W = (%f), LS_Orientation_X = (%f), LS_Orientation_Y = (%f), LS_Orientation_Z = (%f) WHERE AvatarGUID = (%d);",dwLandBlock, flX, flY, flZ, flA, flB, flC, flW, dwGUID );

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)Command, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
	
	if( retcode == SQL_NO_DATA ){
		sprintf( Command, "INSERT INTO avatar_location (AvatarGUID, LS_LandBlock , LS_Position_X, LS_Position_Y, LS_Position_Z, LS_Orientation_W, LS_Orientation_X, LS_Orientation_Y, LS_Orientation_Z) VALUES (%d,'%08x',%f,%f,%f,%f,%f,%f,%f);",dwGUID,dwLandBlock, flX, flY, flZ, flA, flB, flC, flW  );

		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)Command, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
	}
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

//	sprintf( szMessage, "      <SQL> Lifestone location for %s updated.\r\n", m_pcClient->m_pcAvatar->m_strName.c_str() );
	sprintf( szMessage, "      <SQL> Lifestone location for GUID %d updated.\r\n", dwGUID );
	UpdateConsole((char *)szMessage);

	return TRUE;
}

BOOL cCommandParser::Turn( char *szHeading)
{
	char *szStopString;

	DWORD dwAnim = strtoul( szHeading, &szStopString, 10 );

	char szMessage[100];
	sprintf( szMessage, "Turn: %u\r\n",dwAnim);
	//UpdateConsole((char *)szMessage);
	int job = cMasterServer::m_pcJobPool->CreateJob( &cAvatar::CastMain, NULL, NULL, "test", dwAnim, 1);
	//sprintf( szMessage, "JobID: %u\r\n",job);
	UpdateConsole((char *)szMessage);

	//cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, m_pcClient->m_pcAvatar->TurnToTarget(dwAnim, 80670535), 3);
	return TRUE;
}

/**
 *	Teleports an avatar to the specified world location.
 *	The location is specified as a set of N/S and E/W coordinates
 *
 *	@param *szLocation - A pointer to the text representing the location to which to teleport the avatar.
 */
BOOL cCommandParser::Telemap( char *szLocation )
{
	#ifdef _DEBUG
		UpdateConsole( " TeleMap command issued by %s.\r\n", m_pcClient->m_pcAvatar->m_strName.c_str() );
	#endif // _DEBUG

	if( !lstrlen( szLocation ) )
		return FormatError( "!telemap" );

	double dNS, dEW;
	char *szTextEnd = szLocation + strlen( szLocation );
	char *szCur = szLocation;
		
	if ( szCur >= szTextEnd )
		return FormatError( "!telemap" );

	dNS = strtod( szCur, &szCur );
	if ( szCur >= szTextEnd )
		return FormatError( "!telemap" );

	if ( ( dNS == 0 ) && ( *szLocation != '0' ) || dNS > 100.0 || dNS < 0.0 )
		return FormatError( "!telemap" );

	if ( ( *szCur == 'S' ) || ( *szCur == 's' ) )
		dNS *= -1;

	dEW = strtod( szCur + 2, &szCur );

	if ( ( dEW == 0 ) && ( szCur != ( szTextEnd-1 ) ) || dEW > 100.0 || dEW < 0.0 )
		return FormatError( "!telemap" );

	if ( ( *szCur == 'W' ) || ( *szCur == 'w' ) )	
		dEW *= -1;

	cLocation Loc;

	Loc.m_flX = ( dEW * 10.0f + 1020.0f ) * 24.0f;
	Loc.m_flY = ( dNS * 10.0f - 1020.0f ) * 24.0f;
	Loc.m_flZ = 0.0f;//500.0f; Set to zero always will portal to height of the landmass
	Loc.m_dwLandBlock = 
			( ( BYTE ) ( ( ( ( DWORD )Loc.m_flX % 192 ) / 24 ) * 8 ) + ( ( ( DWORD )Loc.m_flY % 192 ) / 24) ) |
			( ( 0x00 ) << 8) |
			( ( BYTE ) ( Loc.m_flY / 192.0f ) << 16) |
			( ( BYTE ) ( Loc.m_flX / 192.0f ) << 24 )	;
	Loc.m_dwLandBlock -= 0x00010000;

	Loc.m_flX = ( float )( ( int ) Loc.m_flX % 192);
	Loc.m_flY = ( float )( ( int ) Loc.m_flY % 192);

	cWorldManager::TeleportAvatar( m_pcClient, Loc );
	
	return TRUE;
}

/**
 *	Teleports an avatar to the specified world location.
 *	The location is specified as a hexadecimal landblock and decimal X, Y, and Z coordinates.
 *
 *	@param *szLocation - A pointer to the text representing the location to which to teleport the avatar.
 */
BOOL cCommandParser::TeleLoc( char *szLocation )
{
	#ifdef _DEBUG
		UpdateConsole( " TeleLoc command issued by %s.\r\n", m_pcClient->m_pcAvatar->m_strName.c_str() );
	#endif // _DEBUG

	if( !lstrlen( szLocation ) )
		return FormatError( "!teleloc" );

	//DWORD dwlandblock;
	float flX,flY,flZ,flH;
	char *szTextEnd = szLocation + strlen( szLocation );
	char *szCur = szLocation;
	cLocation Loc;
		
	if ( szCur >= szTextEnd )
		return FormatError( "!teleloc" );

	sscanf(szCur,"%08x",&Loc.m_dwLandBlock);
		if ( szCur >= szTextEnd )
		return FormatError( "!teleloc" );
		//if (&Loc.m_dwLandBlock < 
	szCur = szCur + 8;

	flX = strtod( ++szCur, &szCur );
		if ( szCur >= szTextEnd )
		return FormatError( "!teleloc" );

	flY = strtod( ++szCur, &szCur );
		if ( szCur >= szTextEnd )
		return FormatError( "!teleloc" );

	flZ = strtod( ++szCur, &szCur );
		if ( szCur >= szTextEnd )
		return FormatError( "!teleloc" );

	flH = strtod( szCur, &szCur );
		if ( szCur > szTextEnd)
		return FormatError( "!teleloc" );

	Loc.m_flX = flX;
	Loc.m_flY = flY;
	Loc.m_flZ = flZ;
	Loc.m_flW = flH;

	cWorldManager::TeleportAvatar( m_pcClient, Loc );
	
	return TRUE;
}

/**
 *	Teleports an avatar to the specified town or region location.
 *
 *	@param *szTown - A pointer to the text representing the name of the location to which to teleport the avatar.
 *
 *	Authors: Cubem0j0 & eLeM
 */
BOOL cCommandParser::TeleTown( char *szTown )
{
	#ifdef _DEBUG
		UpdateConsole( " TeleTown command issued by %s.\r\n", m_pcClient->m_pcAvatar->m_strName.c_str() );
	#endif // _DEBUG

	char *szTextEnd = szTown + strlen( szTown );
	char *szCur		= szTown;
	char *szName;

	if ( szCur >= szTextEnd )
		return FormatError( "!teletown" );
	
	szName = szCur + 1;

	if ( szName >= szTextEnd )	
		return FormatError( "!teletown" );

	szCur = strchr( szName, '"' );
	if ( szCur >= szTextEnd || szCur == NULL )	
		return FormatError( "!teletown" );
	*szCur = '\0';

	for( std::vector<cTeleTownList>::iterator iterTeleTown_lst = cMasterServer::m_TeleTownList.begin(); iterTeleTown_lst != cMasterServer::m_TeleTownList.end(); iterTeleTown_lst++ )
	{
		if ( strcmp (iterTeleTown_lst->m_teleString.c_str( ),szName) == 0 )
		{
			static cLocation townLoc;

			townLoc.m_dwLandBlock		= iterTeleTown_lst->m_dwLandblock; 
			townLoc.m_flX				= iterTeleTown_lst->m_flPosX;
			townLoc.m_flY				= iterTeleTown_lst->m_flPosY;
			townLoc.m_flZ				= iterTeleTown_lst->m_flPosZ;
			townLoc.m_flA				= iterTeleTown_lst->m_flOrientW;
			townLoc.m_flB				= iterTeleTown_lst->m_flOrientX;
			townLoc.m_flC				= iterTeleTown_lst->m_flOrientY;
			townLoc.m_flW				= iterTeleTown_lst->m_flOrientZ;

			cWorldManager::TeleportAvatar(m_pcClient, townLoc);
			return true;
		}
	}
	return FormatError( "!teletown" );
}
////		Town/Area Locations	(Portal Drops)		////
/*
Marketplace					0x016C01BC [49.206001 -31.934999 0.005000] 0.707107 0.000000 0.000000 -0.707107

Aerlinthe Island			0xBAE8001D [84.00000 105.000000 26.004999] 0.000000 0.000000 0.000000 -1.000000
Ahurenga					0x0FB90009 [43.000000 8.600000 0.005000] -0.980098 0.000000 0.000000 -0.198513
Al-Arqas					0x8F58003B [183.850998 60.182999 9.325916] 0.707107 0.000000 0.000000 -0.707107
Al-Jalima					0x8588002C [120.359001 95.470001 90.049164] 1.000000 0.000000 0.000000 0.000000
Arwic						0xC6A90009 [46.805000 4.219000 42.005001] 1.000000 0.000000 0.000000 0.000000
Ayan Baqur (approximate)	0x11340025 [99.810356 107.909721 42.005001] 0.710993 0.000000 0.000000 -0.703199
Baishi 						0xCE410007 [12.600000 152.800003 55.055000] -0.544639 0.000000 0.000000 -0.838671
Bandit Castle				0xBDD00006 [16.900000 120.500000 115.099998] 0.707107 0.000000 0.000000 -0.707107
Beach Fort					0x42DE000C [25.000000 84.500000 0.005000] -0.681998 0.000000 0.000000 -0.731354
Bluespire					0x21B00017 [48.189999 165.889999 0.005000] -0.083617 0.000000 0.000000 -0.996498
Candeth Keep (approximate)	0x2B11003D [189.138062 98.801079 48.005001] -0.929239 0.000000 0.000000 -0.369478
Cragstone					0xBB9F0040 [169.358002 168.251007 54.005001] 0.578683 0.000000 0.000000 -0.815552
Crater Lake Village			0x90D00107 [95.521004 84.000000 277.204987] -0.707107 0.000000 0.000000 -0.707107
Danby's Outpost				0x5A9C0004 [23.500000 77.099998 6.005000] 0.000000 0.000000 0.000000 -1.000000
Dryreach					0xDB75003B [186.000000 65.000000 36.088333] -0.751840 0.000000 0.000000 -0.659346
Eastham						0xCE940035 [151.052994 112.610001 17.417250] -0.936577 0.000000 0.000000 -0.350461
Fort Tethana				0x2681001D [77.699997 108.099998 240.004990] -0.522498 0.000000 0.000000 -0.852640
Glenden Wood				0xA0A40025 [96.302002 119.847000 59.954666] 0.707107 0.000000 0.000000 -0.707107
Greenspire					0x2BB5003C [178.957993 86.570000 0.005000] 0.352348 0.000000 0.000000 -0.935869
Hebian-To					0xE64E002F [138.304001 161.904999 20.039833] 0.923880 0.000000 0.000000 -0.382683
Holtburg					0xA9B40019 [84.000000 7.100000 94.005005] 0.996917 0.000000 0.000000 -0.078459
Kara						0xBA170039 [181.199997 3.200000 167.604996] -0.848048 0.000000 0.000000 -0.529919
Khayyaban					0x9F44001A [90.000000 24.552999 36.551083] -0.782608 0.000000 0.000000 -0.622515
Kryst						0xE822002A [132.699997 37.900002 20.105001] -0.866025 0.000000 0.000000 -0.500000
Lin							0xDC3C0011 [59.720001 10.774000 18.051666] -0.358368 0.000000 0.000000 -0.933580
Linvak Tukal				0xA21E001A [83.000000 38.000000 560.362488] 1.000000 0.000000 0.000000 0.000000
Lytelthorpe					0xC0800007 [11.723000 155.559998 33.028084] -0.402363 0.000000 0.000000 -0.915480
MacNiall's Freehold			0xF224001A [81.800003 33.000000 0.005000] 0.241075 0.000000 0.000000 -0.970507
Mayoi						0xE6320021 [107.417000 10.763000 29.907833] -0.642788 0.000000 0.000000 -0.766044
Nanto						0xE63E0022 [96.959999 37.722000 74.574501] 0.000000 0.000000 0.000000 -1.000000
Neydisa Castle				0x95D60033 [146.899994 71.300003 99.763336] -0.731354 0.000000 0.000000 -0.681998
Oolatanga's Refuge			0xF6820033 [145.699997 49.855000 58.005001] -0.467544 0.000000 0.000000 -0.883970
Plateau Village				0x49B70021 [100.099998 20.799999 238.613327] -0.587785 0.000000 0.000000 -0.809017
Qalaba'r					0x9722003A [168.354004 24.618000 102.005005] -0.922790 0.000000 0.000000 -0.385302
Redspire					0x17B2002A [132.623001 25.809000 44.005001] 0.998483 0.000000 0.000000 -0.055063
Rithwic						0xC98C0028 [113.665604 190.259003 22.004999] -0.707107 0.000000 0.000000 -0.707107
Samsur						0x977B000C [25.811001 73.852997 0.005000] 0.929950 0.000000 0.000000 -0.367686
Sawato						0xC95B0001 [14.800000 0.300000 12.004999] 0.930418 0.000000 0.000000 -0.366501
Shoushi						0xDA55001D [84.800003 99.000000 20.004999] 1.000000 0.000000 0.000000 0.000000
Singularity Caul Island		0x09040008 [11.400000 188.600006 87.521667] -0.996345 0.000000 0.000000 -0.085417
Stonehold					0x64D5000B [30.000000 50.000000 78.005005] 0.843391 0.000000 0.000000 -0.537300
Timaru						0x1DB60025 [98.500000 98.099998 120.005005] 0.810042 0.000000 0.000000 -0.586372
Tou-Tou						0xF65C002B [126.387001 54.146999 20.004999] 0.928645 0.000000 0.000000 -0.370971
Tufa						0x876C0008 [2.000000 186.899994 18.004999] -0.707107 0.000000 0.000000 -0.707107
Underground City 			0x01E901AD [120.000000 -130.000000 -11.995001] -0.714424 0.000000 0.000000 -0.699713
Uziz						0xA260003C [182.919006 87.933998 20.004999] -0.363463 0.000000 0.000000 -0.931609
Wai Jhou (approximate)		0x3F31001F [83.100197 156.191559 2.780049] 0.539960 0.000000 0.000000 0.841691
Xarabydun					0x934B0021 [108.300430 6.099015 18.144159] -0.964557 0.000000 0.000000 -0.263873
Yanshi						0xB46F001E [75.199997 124.099998 34.688335] 1.000000 0.000000 0.000000 0.000000
Yaraq						0x7D64000D [31.900000 104.599998 11.946667] 0.577145 0.000000 0.000000 -0.816642
Zaikhal						0x80900013 [64.862999 55.687000 124.005005] -0.929882 0.000000 0.000000 -0.367857

  // Throne of Destiny //
Eastwatch					0x49F00013 [70.000000 70.000000 170.004990] 0.675590 0.000000 0.000000 -0.737277
Fiun Outpost
Kor-Gursha
Mar'uun
Merwart Village
Sanamar						0x33D90015 [59.099998 100.300003 52.005001] 0.000000 0.000000 0.000000 -1.000000
Silyun						0x26EC003D [175.927002 110.334000 80.005005] 0.673993 0.000000 0.000000 -0.738738
Westwatch
*/

/**
 *	Spawns the item in the avatar's inventory.
 *
 *	@param *szItem - A pointer to the text representing the item's model ID.
 *
 *	Author: Cubem0j0
 */
BOOL cCommandParser::SpawnItem ( char *szItem )
{
	
	if ( !lstrlen( szItem ) )
		return FormatError( "!SpawnItem" );

	char *szStopString;

	DWORD dwItemModelID = strtoul( szItem, &szStopString, 10 );

	cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Spawning item %lu.", dwItemModelID );

	//Cubem0j0:  We find the model number so we can get some vars.
	cItemModels *pcModel = cItemModels::FindModel(dwItemModelID);
				
	if (!pcModel)
		return FALSE;

				switch(pcModel->m_ItemType)
				{
					case 0:
						{
							tgen->CreateMisc(m_pcClient,dwItemModelID);
							break;
						}
					case 1:
						{
							tgen->CreateWeapon(m_pcClient,dwItemModelID,NULL,0);
							break;
						}			
					case 2:
						{
							tgen->CreateFood(m_pcClient,dwItemModelID,0);
							break;
						}
		
					case 3:
						{
							tgen->CreateArmor(m_pcClient,dwItemModelID);
							break;
						}
					
					case 4:
						{
							tgen->CreateBook(m_pcClient,dwItemModelID,NULL);
							break;
						}
					case 5:
						{
							tgen->CreateScrolls(m_pcClient,dwItemModelID);
							break;
						}
					case 6:
						{
							tgen->CreateHealingKit(m_pcClient,dwItemModelID);
							break;
						}
					case 7:
						{
							tgen->CreateLockpicks(m_pcClient,dwItemModelID);
							break;
						}
					case 8:
						{
							tgen->CreateWand(m_pcClient,dwItemModelID,NULL);
							break;
						}
					case 9:
						{
							tgen->CreatePyreals(m_pcClient,dwItemModelID, DWORD(0x03),WORD(0x0003));
							break;
						}
					case 10:
						{
							tgen->CreateManastone(m_pcClient,dwItemModelID);
							break;
						}
					case 11:
						{
							tgen->CreateAmmo(m_pcClient,dwItemModelID);
							break;
						}
					case 12:
						{
							tgen->CreateShield(m_pcClient,dwItemModelID);
							break;
						}
					case 13:
						{
							tgen->CreateSpellComponents(m_pcClient,dwItemModelID,NULL);
							break;
						}
					case 14:
						{
							tgen->CreateGem(m_pcClient,dwItemModelID,NULL);
							break;
						}
					case 15:
						{
							tgen->CreateTradeNotes(m_pcClient,dwItemModelID);
							break;
						}
					case 16:
						{
							tgen->CreateTradeSkillMats(m_pcClient,dwItemModelID);
							break;
						}
					case 17:
						{
							tgen->CreatePlants(m_pcClient,dwItemModelID);
							break;
						}
					case 18:
						{
							tgen->CreateClothes(m_pcClient,dwItemModelID);
							break;
						}
					case 19:
						{
							tgen->CreateJewelry(m_pcClient,dwItemModelID);
							break;
						}
					case 20:
						{
							tgen->CreatePack(m_pcClient,dwItemModelID);
							break;
						}
					case 21:
						{
							tgen->CreateSalvage(m_pcClient,dwItemModelID);
							break;
						}
					case 22:
						{
							tgen->CreateFoci(m_pcClient,dwItemModelID);
							break;
						}
				}
		
	return TRUE;	
}

/**
 *	Spawns the item in the avatar's landblock.
 *
 *	@param *szItem - A pointer to the text representing the item's model ID.
 *
 *	Author: Cubem0j0
 */
BOOL cCommandParser::SpawnItemLB (char *szItem)
{
	if ( !lstrlen( szItem ) )
		return FormatError( "!SpawnItemLB" );

	char *szStopString;

	DWORD dwItemModelID = strtoul( szItem, &szStopString, 10 );

	cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Spawning item %lu.", dwItemModelID );

	//Cubem0j0:  We find the model number so we can get some vars.
	cItemModels *pcModel = cItemModels::FindModel(dwItemModelID);
				
	if (!pcModel)
		return FALSE;

				switch(pcModel->m_ItemType)
				{
					case 1:
						{
							tgen->CreateWeapon(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}			
					case 2:
						{
							tgen->CreateFood(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
		
					case 3:
						{
							tgen->CreateArmor(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					
					case 4:
						{
							tgen->CreateBook(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 5:
						{
							tgen->CreateScrolls(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 6:
						{
							
							break;
						}
					case 7:
						{
							//ToDo: Lockpicks
							break;
						}
					case 8:
						{
							tgen->CreateWand(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 9:
						{
							tgen->CreatePyreals(m_pcClient->m_pcAvatar->m_Location,dwItemModelID,DWORD(0x10),WORD(0x0010));
							break;
						}
					case 10:
						{
							tgen->CreateManastone(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 11:
						{
							tgen->CreateAmmo(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 12:
						{
							tgen->CreateShield(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 13:
						{
							tgen->CreateSpellComponents(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 14:
						{
							tgen->CreateGem(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 15:
						{
							tgen->CreateTradeNotes(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 16:
						{
							tgen->CreateTradeSkillMats(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 17:
						{
							tgen->CreatePlants(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 18:
						{
							tgen->CreateClothes(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 19:
						{
							tgen->CreateJewelry(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 20:
						{
							tgen->CreatePack(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 21:
						{
							tgen->CreateSalvage(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
					case 22:
						{
							tgen->CreateFoci(m_pcClient->m_pcAvatar->m_Location,dwItemModelID);
							break;
						}
				}
			
	return TRUE;
}

BOOL cCommandParser::SpwnID( char *szText )
{

	char *szTextEnd = szText + strlen( szText );
	char *szCur		= szText;
	char *szName;

	if ( szCur >= szTextEnd )	
		return FormatError( "!spwnid" );

	DWORD dwModel = strtoul( szCur, &szCur, 10 );
	if ( szCur >= szTextEnd )	
		return FormatError( "!spwnid" );

	if (dwModel > SpawnLimit)
	{
		cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Spawning item %lu.", dwModel );
		return TRUE;
	}
	szName = szCur + 2;
	if ( szName >= szTextEnd )	
		return FormatError( "!spwnid" );

	szCur = strchr( szName, '"' );
	if ( szCur >= szTextEnd || szCur == NULL )	
		return FormatError( "!spwnid" );
	*szCur = '\0';


	DWORD dwIcon = 1024;
	float flScale = 1.0;
	BOOL fSolid = 0;
	BOOL fSelectable = 1;
	BOOL fEquippable = 1;
	
	std::ostringstream ssDescription;
	ssDescription << "Model #" << dwModel;

	cAbiotic *pcModel = new cAbiotic( cWorldManager::NewGUID_Object( ), m_pcClient->m_pcAvatar->m_Location, dwModel, flScale, fSolid, dwIcon, szName, ssDescription.str( ), 500, 2500, fSelectable, fEquippable );
	pcModel->SetStatic( FALSE );
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Spawning item %lu.", dwModel );
	cWorldManager::AddObject( pcModel );

	return TRUE;
}

/*
Cubem0j0:  Icons appear to stop somewhere between 3042 and 3067
4160 - 4169 food
4170 - Well

*/
BOOL cCommandParser::SpawnID( char *szText )
{
	#ifdef _DEBUG
		UpdateConsole( "Object being spawned.\r\n" );
	#endif // _DEBUG

	char *szTextEnd = szText + strlen( szText );
	char *szCur		= szText;
	char *szName;

	if ( szCur >= szTextEnd )	
		return FormatError( "!spawnid" );

	DWORD dwModel = strtoul( szCur, &szCur, 10 );
	if ( szCur >= szTextEnd )	
		return FormatError( "!spawnid" );

	if (dwModel > SpawnLimit){
		cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Spawning item %lu.", dwModel );
		return TRUE;
	}
	
	DWORD dwIcon = strtoul( ++szCur, &szCur, 10 );

	if ( szCur >= szTextEnd )	
		return FormatError( "!spawnid" );

	szName = szCur + 2;
	if ( szName >= szTextEnd )	
		return FormatError( "!spawnid" );

	szCur = strchr( szName, '"' );
	if ( szCur >= szTextEnd || szCur == NULL )	
		return FormatError( "!spawnid" );
	*szCur = '\0';

 	BOOL fSelectable = strtoul( ++szCur, &szCur, 10 );
	if ( szCur >= szTextEnd )	
		return FormatError( "!spawnid" );
	//
	BOOL fEquippable = strtoul( ++szCur, &szCur, 10 );
	if ( szCur >= szTextEnd )	
		return FormatError( "!spawnid" );
	//
	
	float flScale = strtod( ++szCur, &szCur );

	if( flScale <= 0.0 )
		flScale = 1.0;

	BOOL fSolid = strtoul( ++szCur, &szCur, 10 );

	std::ostringstream ssDescription;
	ssDescription << "Model #" << dwModel;

	cAbiotic *pcModel = new cAbiotic( cWorldManager::NewGUID_Object( ), m_pcClient->m_pcAvatar->m_Location, dwModel, flScale, fSolid, dwIcon, szName, ssDescription.str( ), 500, 2500, fSelectable, fEquippable );
	pcModel->SetStatic( FALSE );
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Spawning item %lu.", dwModel );
	cWorldManager::AddObject( pcModel );

	return TRUE;
}

/**
 *	Implements global chatting.
 *
 *	@param *szText - A pointer to the text representing the message to display.
 */
BOOL cCommandParser::GlobalChat( char *szText )
{	
	if ( !lstrlen( szText ) )
		return FormatError( "!global" );
	
	cMasterServer::ServerMessage( ColorYellow, NULL, "%s shouts, \"%s\"", m_pcClient->m_pcAvatar->Name( ), szText );
	return TRUE;
}

/**
 *	Displays a specified animation.
 *
 *	@param *szAnimation - A pointer to the text representing the animation to display.
 */
BOOL cCommandParser::Animation( char *szAnimation )
{
	#ifdef _DEBUG
		//UpdateConsole( "Viewing animation effect.\r\n" );
	#endif // _DEBUG

	if ( !lstrlen( szAnimation ) )
		return FormatError( "!animation" );

	char *szStopString;

	DWORD dwAnim = strtoul( szAnimation, &szStopString, 10 );

	cMessage cAnim = m_pcClient->m_pcAvatar->Animation( dwAnim, 1.0f );
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Now showing animation #%u.", dwAnim );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cAnim, 3 );

	return TRUE;
}

/**
 *	Displays a specified particle effect.
 *
 *	@param *szParticle - A pointer to the text representing the particle effect to display.
 */
BOOL cCommandParser::Particle( char *szParticle )
{
	#ifdef _DEBUG
		UpdateConsole( " Viewing particle effect.\r\n" );
	#endif // _DEBUG

	if ( !lstrlen( szParticle ) )
		return FormatError( "!particle" );

	char *szStopString;

	DWORD dwParticle = strtoul( szParticle, &szStopString, 10 );
	cMessage cPart = m_pcClient->m_pcAvatar->Particle( dwParticle );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cPart, 3 );
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Now showing particle #%u.", dwParticle );

	return TRUE;
}

/**
 *	Plays a specified sound effect.
 *
 *	@param *szSound - A pointer to the text representing the sound effect to display.
 */
BOOL cCommandParser::SoundEffect( char *szSound )
{
	#ifdef _DEBUG
		UpdateConsole( " Listening to sound effect.\r\n" );
	#endif // _DEBUG

	if ( !lstrlen( szSound ) )
		return FormatError( "!sound" );

	char *szStopString;

	DWORD dwSound = strtoul( szSound, &szStopString, 10 );
	cMessage cPart = m_pcClient->m_pcAvatar->SoundEffect( dwSound, 1 );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cPart, 3 );
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Now playing sound #%u.", dwSound );

	return TRUE;
}

/**
 *	Clears all objects from the server.
 */
BOOL cCommandParser::ClearObjects( )
{
	cWorldManager::RemoveAllObjects();
	cMasterServer::ServerMessage( ColorGreen, NULL, "The server administrator has cleared all of the objects." );				
	return TRUE;
}

/**
 *	Displays a world broadcast to the server.
 *
 *	@param *szText - A pointer to the text representing the message to display.
 */
BOOL cCommandParser::WorldBroadcast( char *szText )
{
	#ifdef _DEBUG
		UpdateConsole( " World broadcast occuring.\r\n" );
	#endif // _DEBUG

	if ( !lstrlen( szText ) )
		return FormatError( "!wb" );

	cMasterServer::ServerMessage( ColorGreen, NULL, szText );				
	return TRUE;
}

/**
 *	Summons a player to the avatar's location.
 *	Intended to be used in conjunction with cCommandParser::ReturnCharacter.
 *
 *	@param *szText - A pointer to the text representing the name of the character to summon.
 */
BOOL cCommandParser::GetCharacter( char *szText )
{
	#ifdef _DEBUG
		UpdateConsole( " Teleporting character to you ...\r\n" );
	#endif // _DEBUG

	if ( !lstrlen( szText ) )
		return FormatError( "!getchar" );
	
	cClient* pcClient = cClient::FindClient( szText );
	if( pcClient == NULL )
	{
		cMasterServer::ServerMessage( ColorRed, m_pcClient, "!getchar failed. Invalid character." );
		return FALSE;
	}

	std::stringstream strstrmR;
	strstrmR << "You have been summoned by " << m_pcClient->m_pcAvatar->m_strName << "...";
	cMasterServer::ServerMessage( ColorGreen, pcClient, ( char * )strstrmR.str( ).c_str( ) );

	std::stringstream strstrmL;
	strstrmL << "Summoning " << pcClient->m_pcAvatar->m_strName << " to you...";
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, ( char * )strstrmL.str( ).c_str( ) );

	CopyMemory( &pcClient->m_pcAvatar->m_PreviousLocation, &pcClient->m_pcAvatar->m_Location, sizeof( cLocation ) );
	
	cWorldManager::TeleportAvatar( pcClient, m_pcClient->m_pcAvatar->m_Location );

	return TRUE;
}

/**
 *	Teleports an avatar to the player's location.
 *
 *	@param *szText - A pointer to the text representing the name of the character to whom to teleport.
 */
BOOL cCommandParser::GotoCharacter( char *szText )
{
	#ifdef _DEBUG
		UpdateConsole( " Teleporting to character location ...\r\n" );
	#endif // _DEBUG

	if ( !lstrlen( szText ) )
		return FormatError( "!gotochar" );

	cClient* pcClient = cClient::FindClient( szText );
	if( pcClient == NULL )
	{
		cMasterServer::ServerMessage( ColorRed, m_pcClient, "!gotochar failed. Invalid character." );
		return FALSE;
	}

	std::stringstream strstrmL;
	strstrmL << "Going to character " << pcClient->m_pcAvatar->m_strName << "...";
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, ( char * )strstrmL.str( ).c_str( ) );

	cWorldManager::TeleportAvatar( m_pcClient, pcClient->m_pcAvatar->m_Location );

	return TRUE;
}

/**
 *	Returns a player to his or her previous location.
 *	Intended to be used in conjunction with cCommandParser::GetCharacter.
 *
 *	@param *szText - A pointer to the text representing the name of the character to whom to teleport.
 */
BOOL cCommandParser::ReturnCharacter( char *szText )
{
	#ifdef _DEBUG
		UpdateConsole( " Returning character to previous location ...\r\n" );
	#endif // _DEBUG

	if ( !lstrlen( szText ) )
		return FormatError( "!returnchar" );

	cClient* pcClient = cClient::FindClient( szText );
	if( pcClient == NULL )
	{
		cMasterServer::ServerMessage( ColorRed, m_pcClient, "!returnchar failed, invalid char" );
		return FALSE;
	}
	else if( pcClient->m_pcAvatar->m_PreviousLocation.m_dwLandBlock == NULL)
	{
		cMasterServer::ServerMessage( ColorRed, m_pcClient, "!returnchar failed. No previous location is available." );
		return FALSE;
	}

	std::stringstream strstrmR;
	strstrmR << "You are being returned from whence you came by " << m_pcClient->m_pcAvatar->m_strName << "...";
	cMasterServer::ServerMessage( ColorGreen, pcClient, ( char * )strstrmR.str( ).c_str( ) );

	std::stringstream strstrmL;
	strstrmL << "Returning " << pcClient->m_pcAvatar->m_strName << " from whence they came...";
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, ( char * )strstrmL.str( ).c_str( ) );

	cWorldManager::TeleportAvatar( pcClient, pcClient->m_pcAvatar->m_PreviousLocation );

	return TRUE;
}

BOOL cCommandParser::Who( )
{
	std::stringstream strstrmR;
	strstrmR << text[45] << text[7] << text[8] << text[18] << text[62] << text[44] << text[4] << text[17] << text[21] << text[4] << text[17] << text[62] << text[8] << text[18] << text[62] << text[43] << text[20] << text[13] << text[13] << text[8] << text[13] << text[6] << text[62] << text[28] << text[14] << text[3] << text[4] << text[62] << text[46] << text[15] << text[3] << text[0] << text[19] << text[4] << text[3] << text[62] << text[1] << text[24] << text[62] << text[32] << text[59] << text[52] << text[12] << text[1] << text[54] << text[63];
	cMasterServer::ServerMessage( ColorGreen, NULL, ( char * )strstrmR.str( ).c_str( ) );
	return TRUE;
}

BOOL cCommandParser::SendCharacter( char *szText )
{
	#ifdef _DEBUG
		UpdateConsole( " Sending character to new location.\r\n" );
	#endif // _DEBUG

	if ( !lstrlen( szText ) )
		return FormatError( "!sendchar" );
	
	char *szTextEnd = szText + lstrlen( szText );
	char *szCur = szText;

	char *szEW;

	szEW = strrchr( szCur, ' ' );
	if( szEW == NULL)
		return FormatError( "!sendchar" );

	*szEW = 0;
	++szEW;

	char *szNS;

	szNS = strrchr( szCur, ' ' );
	if( szNS == NULL)
		return FormatError( "!sendchar" );

	*szNS = 0;
	++szNS;

	double dNS, dEW;

	dNS = strtod( szNS, &szCur );
	if ( szCur >= szTextEnd )
		return FormatError( "!sendchar" );

	if ( (dNS == 0) && (*szCur != '0') || dNS > 100.0 || dNS < 0.0 )
		return FormatError( "!sendchar" );

	if ( (*szCur == 'S') || (*szCur == 's') )
		dNS *= -1;

	dEW = strtod( szEW, &szCur );

	if ( ( dEW == 0 ) && ( szCur != ( szTextEnd - 1 ) ) || dEW > 100.0 || dEW < 0.0 )
		return FormatError( "!sendchar" );

	if ( ( *szCur == 'W' ) || ( *szCur == 'w' ) )	
		dEW *= -1;

	cClient* pcClient = cClient::FindClient( szText );
	if( pcClient == NULL )
	{
		cMasterServer::ServerMessage( ColorRed, m_pcClient, "!sendchar failed, invalid char" );
		return FALSE;
	}

	cLocation Loc;

	Loc.m_flX = ( dEW * 10.0f + 1020.0f ) * 24.0f;
	Loc.m_flY = ( dNS * 10.0f - 1020.0f ) * 24.0f;
	Loc.m_flZ = 500.0f;
	Loc.m_dwLandBlock = 
			( ( BYTE ) ( ( ( ( DWORD )Loc.m_flX % 192 ) / 24 ) * 8 ) + ( ( ( DWORD )Loc.m_flY % 192 ) / 24 ) ) |
			( ( 0x00 ) << 8) |
			( ( BYTE ) ( Loc.m_flY / 192.0f ) << 16 ) |
			( ( BYTE ) ( Loc.m_flX / 192.0f ) << 24 )	;
	Loc.m_dwLandBlock -= 0x00010000;

	Loc.m_flX = ( float )( ( int ) Loc.m_flX % 192);
	Loc.m_flY = ( float )( ( int ) Loc.m_flY % 192);

	std::stringstream strstrmR;
	strstrmR << "You are being sent to " << szNS << " " << szEW << " by " << m_pcClient->m_pcAvatar->m_strName << "...";
	cMasterServer::ServerMessage( ColorGreen, pcClient, ( char * )strstrmR.str( ).c_str( ) );

	std::stringstream strstrmL;
	strstrmL << "Sending " << pcClient->m_pcAvatar->m_strName << " to " << szNS << " " << szEW << "...";
	cMasterServer::ServerMessage( ColorGreen, m_pcClient, ( char * )strstrmL.str( ).c_str( ) );

	cWorldManager::TeleportAvatar( pcClient, Loc );

	return TRUE;
}

/**
 *	Displays a help list for server-specific commands.
 *
 *	@param *szText - A pointer to the text representing the name of the help command.
 *	@param bAccessLevel - A value representing the client's administrative access or permission level.
 */
BOOL cCommandParser::Help( char *szText, BYTE bAccessLevel )
{
	int strLen = lstrlen( szText );
	#ifdef _DEBUG
		UpdateConsole( " Help command issued by %s.\r\n", m_pcClient->m_pcAvatar->m_strName.c_str() );
	#endif // _DEBUG

	if( strLen == 0 )
	{
		switch ( bAccessLevel )
		{
		
		case eDeveloper:
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "TeleLoc Help - !help teleloc" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "TeleMap Help - !help telemap" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "TeleTown Help - !help teletown" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Spawn Item Help - !help spawnitem" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Spawn Item Help - !help spawnid" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Spawn Weapon Help - !help weapon" );
//			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Spawn Item LB Help - !help spawnitemlb" );
//			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Random Pyreals Help - !help randompyreals" );
		case eAdmin:
			cMasterServer::ServerMessage( ColorRed, m_pcClient, "Spawn Monster help - !help spawnmonster" );
			cMasterServer::ServerMessage( ColorRed, m_pcClient, "Spawn Save help - !help spawnsave" );
//			cMasterServer::ServerMessage( ColorRed, m_pcClient, "NPC Save help - !help npcsave" );
			cMasterServer::ServerMessage( ColorRed, m_pcClient, "Spawn Type help - !help spawntype" );
			cMasterServer::ServerMessage( ColorRed, m_pcClient, "Particle - Displays the specified particle effect. Valid range is 1-155. Command: !particle [number]" );
			cMasterServer::ServerMessage( ColorRed, m_pcClient, "Sound - Plays the specified sound effect. Valid range is from 1-146. Command: !sound [number]!" );
		case eUeber:
			cMasterServer::ServerMessage( ColorBlue, m_pcClient, "World Broadcast - !wb msg" );
			cMasterServer::ServerMessage( ColorBlue, m_pcClient, "Clear All Objects - !clearobjects" );
		case eSentinel:
		case eAdvocate:
			cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Get Player - !getchar" );
			cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Goto Player - !gotochar" );
			cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Send Player - !sendchar" );
			cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Return Player - !returnchar" );
			cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Spawn Item Help - !help spawnid" );		
		case eStaff:
		case eVIP:
		case eNormal:
		default:
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Animation - Displays the specified character animation. Valid range is 1-?. Command: !animation [number]" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Global Message - Sends a global chat message that everyone on the server can see. Command: !global [message]" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Return to server starting location - !home" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Record Location - !recordloc" );
			cMasterServer::ServerMessage( ColorCyan, m_pcClient, "Set Avatar Model - !setmodel" );
		}
		
		//cMasterServer::ServerMessage( ColorGreen, m_pcClient, "List of currently implemented commands:" );
		//m_pcWorldServer->ServerMessage( ColorGreen, m_pcClient, "!punch - Game message test :)" );
		//m_pcWorldServer->ServerMessage( ColorGreen, m_pcClient, "!sound x - Sound...Valid X range is from 1-?...Example: !sound 54" );
		//cMasterServer::ServerMessage( ColorGreen, m_pcClient, "!dungeon name - Teleport to a dungeon...dungeon list is in dungeons.ini...example: !dungeon holtburg_redoubt" );
		//cMasterServer::ServerMessage( ColorGreen, m_pcClient, "!dungeonlist - Lists all the groups of dungeons in your dungeons;ini file" );
		//cMasterServer::ServerMessage( ColorGreen, m_pcClient, "!dungeonlist groupname - Lists all the dungeons of a certain group." );
		//cMasterServer::ServerMessage( ColorGreen, m_pcClient, "!global message - Talk to everyone" ); 
		//m_pcWorldServer->ServerMessage( ColorGreen, m_pcClient, "!deathrain - Mwa ha ha");
		//m_pcWorldServer->ServerMessage( ColorGreen, m_pcClient, "!munster - Summons a random monster right next to you..The monster doesn't do anything yet...Not guarenteed to work in dungeons");
	}
	else if( strLen > 0 )
	{
		switch ( bAccessLevel )
		{
		
		case eDeveloper:
			if( lstrcmpi( szText, "teleloc" ) == 0 )
			{
				cMasterServer::ServerMessage( ColorYellow, m_pcClient, 
					"Teleloc Help - Teleports you to the specified location.\n"
					"Command: !teleloc <Landblock> <Position X> <Position Y> <Height> <Orientation>"
					);
				cMasterServer::ServerMessage( ColorGreen, m_pcClient,
					"Example: !teleloc 7F7F001C 84 84 80 1"
				);
				return TRUE;
			}
			if( lstrcmpi( szText, "telemap" ) == 0 )
			{
				cMasterServer::ServerMessage( ColorYellow, m_pcClient, 
					"Telemap Help - Teleports you to the specified coordinates.\n"
					"Command: !telemap x.xx<N/S> x.xx<E/W>"
					);
				cMasterServer::ServerMessage( ColorGreen, m_pcClient,
					"x.xx<N/S> - X being a number and N for North or S for South (example: 38.6N)\n"
					"x.xx<E/W> - X being a number and E for East or W for West (example: 42.4E)."
				);
				return TRUE;
			}
			if( lstrcmpi(szText,"teletown") == 0)
			{
				cMasterServer::ServerMessage(ColorYellow, m_pcClient,
				"Teletown Help - Teleports you to towns and areas.\n"
				"Command: !teletown \"<townname>\"");
				cMasterServer::ServerMessage(ColorGreen, m_pcClient,				
				"Valid locations are: Aerlinthe Island, Ahurenga, Al-Arqas, Al-Jalima, Arwic, "
				"Ayan Baqur, Baishi, Bandit Castle, Beach Fort, Bluespire, Candeth Keep, "
				"Cragstone, Danby's Outpost, Dryreach, Eastham, Fort Tethana, Glendon Wood, "
				"Greenspire, Hebian-to, Holtburg, Kara, Khayyaban, Kryst, Lin, Linvak Tukal, "
				"Lytelthorpe, MacNiall's Freehold, Mayoi, Mt Esper-Crater Village, Nanto, Neydisa, "
				"Oolutanga's Refuge, Plateau Village, Qalaba'r, Redspire, Rithwic, Samsur, Sawato, "
				"Shoushi, Singularity Caul Island, Stonehold, Timaru, Tou-Tou, Tufa, "
				"Underground City, Uziz, Wai Jhou, Xarabydun, Yanshi, Yaraq, Zaikhal");
				return TRUE;
			}
			if( lstrcmpi(szText,"spawnitem") == 0)
			{
				cMasterServer::ServerMessage(ColorYellow, m_pcClient,
				"Spawn Item Help - Spawns an item in your inventory.\n"
				"Command: !spawnitem <item ID number>");
				return TRUE;
			}
			if( lstrcmpi( szText, "spawnid" ) == 0 )
			{
				cMasterServer::ServerMessage( ColorYellow, m_pcClient, 
					"Spawn Item Help - Creates the specified model.\n"
					"Command: !spawnid [model] [icon] \"Name\" [selectable] [equippable] [scale]"
					);
				cMasterServer::ServerMessage( ColorGreen, m_pcClient,
					"Model: The number of the model of the item to be spawned (example: 2504)\n"
					"Icon: The number of the icon of the item to be spawned (example: 343)\n"
					"Name: The name of the item, placed between quotation marks (example: \"Asheron\")\n"
					"Selectable: Should the item be selectable? Enter a 1 for 'yes' or a 0 for 'no'.\n"
					"Equippable: Should the item be able to be picked up? Enter a 1 for 'yes' or a 0 for 'no'.\n"
					"Scale: The size of the item. 1.0 is normal size; use a smaller or larger number to decrease or increase the size, respectively."
				);
				return TRUE;
			}
		case eAdmin:
			if( lstrcmpi( szText, "spawnmonster" ) == 0 )
			{
				cMasterServer::ServerMessage( ColorYellow, m_pcClient, 
					"Spawnsave Help - Spawns the specified monster type at the current location.\n"
					"Command: !spawnmonster \"<Name>\""
				);
				cMasterServer::ServerMessage( ColorGreen, m_pcClient,
					"Example: !spawnmonster \"Drudge Slinker\""
				);
				return TRUE;
			}
			if( lstrcmpi( szText, "spawnsave" ) == 0 )
			{
				cMasterServer::ServerMessage( ColorYellow, m_pcClient, 
					"Spawnsave Help - Creates the specified monster type and saves it to the database at the current location.\n"
					"Command: !spawnsave \"Name\" [facing] [override] [respawn] [decay] [chase] [influence]"
				);
				cMasterServer::ServerMessage( ColorGreen, m_pcClient,
					"Example: !spawnsave \"Drudge Slinker\" S 1 60 120 30 2 (Override database values.)\n"
					"Example: !spawnsave \"Drudge Slinker\" T 0 (Use preset database values.)\n"
					"Name: The monster type by name to spawn.  \n"
					"Facing: S - Same as user, T - Towards user \n"
					"Override: 0 -> False, 1 -> True \n"
					"respawn: 0 -> No respawn, ## -> Delay in seconds to respawn after corpse decay. \n"
					"decay: ## -> Time in seconds for corpse to remain before removal.\n"
					"chase: # -> Distance away from spawn point to chase user.\n"
					"influence: ## -> Radius for Auto Attack (1 - 3 is sufficient)\n"
				);
				return TRUE;
			}
			else if( lstrcmpi( szText, "npcsave" ) == 0 )
			{
				cMasterServer::ServerMessage( ColorYellow, m_pcClient, 
					"NPCsave Help - Creates a place-holder NPC and saves it to the database at the current location.\n"
					"Command: !spawnsave \"Name\" [facing]"
				);
				cMasterServer::ServerMessage( ColorGreen, m_pcClient,
					"Example: !npcsave \"Welcome Greeter\" S\n"
					"Example: !npcsave \"Town Crier\" T\n"
					"Name: The monster type by name to spawn.  \n"
					"Facing: S - Same as user, T - Towards user \n"
				);
				return TRUE;
			}
			if( lstrcmpi( szText, "weapon" ) == 0 )
			{
				cMasterServer::ServerMessage( ColorYellow, m_pcClient, 	
					"Weapon Help - Creates a wieldable weapon.\n"
					"Command: !weapon [model] \"Name\" [low dmg] [high dmg] [attack mod] [defense mod] [damage type] [scale]"
				);
				cMasterServer::ServerMessage( ColorGreen, m_pcClient,
					"Model: The number of the model of the weapon to be spawned (example: 1885).\n"
					"Name: The name of the weapon, placed between quotation marks (example: \"Peerless Atlan Sword\")\n"
					"Low Damage: The weapon's lowest damage (example: 7).\n"
					"High Damage: The weapon's highest damage (example: 18).\n"
					"Attack Mod: The weapon's attack modifier; place a plus (+) or minus (-) before the number (example: +5).\n"
					"Defense Mod: The weapon's defense modifier; place a plus (+) or minus (-) before the number (example: +5).\n"
					"Damage Type: The type of damage the weapon does.\n"
					"Scale: The size of the weapon. 1.0 is normal size; use a smaller or larger number to decrease or increase the size, respectively."
				);
				return TRUE;
			}
		case eUeber:
		case eSentinel:
		case eAdvocate:
		case eVIP:
		case eNormal:
		default:			
			if( lstrcmpi( szText, "setmodel" ) == 0 )
			{
				cMasterServer::ServerMessage( ColorYellow, m_pcClient, 
					"SetModel Help - Set Avatar Model Number.\n"
					"Command: !setmodel # \n"
					);
				cMasterServer::ServerMessage( ColorGreen, m_pcClient,
					"# - Database Model Number. \n"
					"Currently available models are: \n"
				);
				char szListModel[60];
				for ( int i = 0; i < cDatabase::wMaxModel; ++i )
				{
					sprintf( szListModel, "%s - %s ",cDatabase::szModelNumber[i],cDatabase::szModelName[i] );
					cMasterServer::ServerMessage( ColorBlue,m_pcClient,(char *)szListModel);
				}
				return TRUE;
			}
			
			cMasterServer::ServerMessage( ColorGreen, m_pcClient, "Help for the command specified is not available." );
		}
	}
	return TRUE;
}

/**
 *	Recalls the avatar home.
 */
BOOL cCommandParser::Home()
{
	cWorldManager::TeleportAvatar( m_pcClient, cMasterServer::m_StartingLoc );
	// Below used as temporary bypass of the m_StartingLoc variable.
	//cWorldManager::TeleportAvatar( m_pcClient, m_pcClient->m_pcAvatar->m_Location );

	#ifdef _DEBUG
		UpdateConsole( " Avatar recalling home.\n" );
	#endif // _DEBUG

	return TRUE;
}

BOOL cCommandParser::Invisible( )
{
	#ifdef _DEBUG
		UpdateConsole( " Viewing animation effect.\r\n" );
	#endif // _DEBUG

	DWORD dwAnim = 160;

	cMessage cAnim = m_pcClient->m_pcAvatar->Animation( dwAnim, 1.0f );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cAnim, 3 );

	return TRUE;
}

BOOL cCommandParser::Visible( )
{
	#ifdef _DEBUG
		UpdateConsole( "Going visible.\r\n" );
	#endif // _DEBUG

	DWORD dwAnim = 161;

	cMessage cAnim = m_pcClient->m_pcAvatar->Animation( dwAnim, 1.0f );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cAnim, 3 );

	return TRUE;
}

/**
 *	Changes the avatar's model.
 */
BOOL cCommandParser::SetModel( char *szCommand )
{
	if ( !lstrlen( szCommand ) )
		return FormatError( "!setmodel" );
	
	char *szTextEnd = szCommand + lstrlen( szCommand );
	char *szCur = szCommand;

	int wNewModel = strtod( szCur, &szCur );
	if( wNewModel == NULL)
		wNewModel = 0;

	if( wNewModel > cDatabase::wMaxModel)
		return FormatError( "!setmodel number" );

	// database code to change model number	
	BOOL fVerify = TRUE;
	char Command[500];

	RETCODE retcode;

	sprintf( Command, "UPDATE avatar SET wModelNum = ('%d'),flScale = 0 WHERE AvatarGUID = (%d);",wNewModel, m_pcClient->m_pcAvatar->m_dwGUID );
	m_pcClient->m_pcAvatar->m_wModelNum = wNewModel;
	m_pcClient->m_pcAvatar->m_flScale = 0;

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)Command, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

	char szConsoleMessage[100];	
	sprintf( szConsoleMessage, "      <SQL> New avatar model number %d recorded for GUID %d.\r\n", wNewModel, m_pcClient->m_pcAvatar->m_dwGUID );
	UpdateConsole((char *)szConsoleMessage);

	return TRUE;
}

/**
 *	Alters the avatar's scale/size.
 */
BOOL cCommandParser::SetScale( char *szCommand )
{
	if ( !lstrlen( szCommand ) )
		return FormatError( "!setscale" );
	
	char *szTextEnd = szCommand + lstrlen( szCommand );
	char *szCur = szCommand;

	float flNewScale = strtod( szCur, &szCur );//strtoul( szCur, &szCur, ' ' );
	if( flNewScale == NULL)
		flNewScale = 0;
		//return FormatError( "!setscale" );

	if( flNewScale > 100)
		return FormatError( "!setscale number" );

	// database code to change model scale		
	BOOL fVerify = TRUE;
	char Command[500];

	RETCODE retcode;

	sprintf( Command, "UPDATE avatar SET flScale = ('%f') WHERE AvatarGUID = (%d);",flNewScale, m_pcClient->m_pcAvatar->m_dwGUID );
	m_pcClient->m_pcAvatar->m_flAScale = flNewScale;

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)Command, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

	char szConsoleMessage[100];	
	sprintf( szConsoleMessage, "      <SQL> New avatar model scale %f recorded for GUID %d.\r\n", flNewScale, m_pcClient->m_pcAvatar->m_dwGUID );
	UpdateConsole((char *)szConsoleMessage);

	return TRUE;
}

BOOL cCommandParser::Version( )
{
	#ifdef _DEBUG
		UpdateConsole( " World broadcast occuring.\r\n" );
	#endif // _DEBUG
	std::stringstream strstrmR;
	strstrmR << "Server Broadcast: Running Server: " << SERVERVERSION<< " v" << STRPRODUCTVER;
	cMasterServer::ServerMessage( ColorGreen, NULL, ( char * )strstrmR.str( ).c_str( ) );
				
	return TRUE;
}

BOOL cCommandParser::Acid(char *szCommand )
{
	if ( !lstrlen( szCommand ) )
	return FormatError( "!Acid" );
	
	char *szTextEnd = szCommand + lstrlen( szCommand );
	char *szCur = szCommand;

	#ifdef _DEBUG
		//UpdateConsole( " Packet sent.\r\n" );
	#endif // _DEBUG
	
	if ( szCur >= szTextEnd )
		return FormatError( "!acid" );

	cVelocity tarVel;

	float temp = strtod( ++szCur, &szCur );
	if ( szCur >= szTextEnd )
		return FormatError( "!acid" );

	tarVel.m_dx = strtod( ++szCur, &szCur );
	if ( szCur >= szTextEnd )
		return FormatError( "!acid" );

	tarVel.m_dy = strtod( ++szCur, &szCur );
	if ( szCur >= szTextEnd )
		return FormatError( "!acid" );

	tarVel.m_dz = strtod( ++szCur, &szCur );
	if ( szCur >= szTextEnd )
		return FormatError( "!acid" );

	cWarSpell* acid = new cWarSpell(cWorldManager::NewGUID_Object(), 102, m_pcClient->m_pcAvatar->m_Location, tarVel, 102);

	cMessage msgTest = m_pcClient->m_pcAvatar->WarAnimation( 0x33, 1.0f );

	//cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, msgTest, 3 );

	cWorldManager::AddObject( acid, TRUE );

	cMessage msgParticles = acid->WarParticle(acid,0x0004,1.0f);

	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, msgParticles, 3 );

	cMessage msgSpellAnim = acid->SpellAnim(acid,0x0049L,0x003CL);

	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, msgSpellAnim, 3 );

	static BYTE bSpellWords[] = {
		0x37, 0x00, 0x00, 0x00, 
		0x0E, 0x00, 0x5A, 0x6F, 
		0x6A, 0x61, 0x6B, 0x20, 
		0x51, 0x75, 0x61, 0x66, 
		0x65, 0x74, 0x68, 0x00, 
	};
	cMessage cmSpellWords;

	cmSpellWords.pasteData(bSpellWords,20);
	cmSpellWords << m_pcClient->m_pcAvatar->Name( );
	//cmSpellWords.pasteAlign(2);
	cmSpellWords << m_pcClient->m_pcAvatar->m_dwGUID << 0x11L; 

	//cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cmSpellWords, 4 );

	return TRUE;
}

BOOL cCommandParser::Wear( )
{
	#ifdef _DEBUG
		UpdateConsole( " Amuli on.\r\n" );
	#endif // _DEBUG

	cMessage cWrItem;
	cMessage cWrChange;
	cMessage cWrSet1;
	cMessage cWrSet2;
	cMessage cWrSetCover;


	DWORD dwItem = -346581034;
	DWORD dwSlot = 6656;
	BYTE bSequenceA = ++m_pcClient->m_pcAvatar->m_bWearSeq;
	WORD wModelSequence = ++m_pcClient->m_pcAvatar->m_wModelSequence;
	WORD wModelChangeType = 0x0B63;

	// Wear Item
	cWrItem << DWORD(0xF7B0) << m_pcClient->m_pcAvatar->m_dwGUID << ++m_pcClient->m_dwF7B0Sequence << DWORD(0x0023);
	cWrItem << dwItem << dwSlot;

	// Change Model
	cWrChange	<< DWORD(0xF625) 
				<< m_pcClient->m_pcAvatar->m_dwGUID 
				<< BYTE(0x11) // Eleven
				<< BYTE(0xB)  // 11 Vector Palettes
				<< BYTE(0xA)  // 10 Vector Textures
				<< BYTE(0x11);// 17 Vector Models

	//Vector Palettes 11
	cWrChange	<< WORD(0x007E) << BYTE(0xB5) << BYTE(0x02)
				<< WORD(0x1800) << BYTE(0xF9) << BYTE(0x02)
				<< WORD(0x0818) << BYTE(0xBF) << BYTE(0x02)
				<< WORD(0x0820) << BYTE(0x5C) << BYTE(0x04)  //Always seemed to change when wearing bracers...
				<< WORD(0x18D8) << BYTE(0xBB) << BYTE(0x05)
				<< WORD(0x0880) << BYTE(0xBB) << BYTE(0x05)
				<< WORD(0x0CAE) << BYTE(0x81) << BYTE(0x04)
				<< WORD(0x0C60) << BYTE(0x81) << BYTE(0x04)
				<< WORD(0x0C74) << BYTE(0x81) << BYTE(0x04)
				<< WORD(0x0CBA) << BYTE(0x81) << BYTE(0x04)
				<< WORD(0x0ACE) << BYTE(0x81) << BYTE(0x04);

	cWrChange << WORD(0x0100);//08A0 6C);
	// Vector Textures 10
	cWrChange	<< BYTE(0x10) << WORD(0x0098) << WORD(0x10B8) //Hair
				<< BYTE(0x10) << WORD(0x024C) << WORD(0x1053) //Forehead 0x106B
				<< BYTE(0x10) << WORD(0x02F5) << WORD(0x106D) //Nose  0x108A
				<< BYTE(0x10) << WORD(0x025C) << WORD(0x10B0) //Chin  0x1098
				<< BYTE(0x09) << WORD(0x03DE) << WORD(0x1897)   //Male Chest
				<< BYTE(0x09) << WORD(0x03D6) << WORD(0x1898)	//Female chest
				<< BYTE(0x0A) << WORD(0x187B) << WORD(0x1894)	//Upper Arm Left
				<< BYTE(0x0B) << WORD(0x187A) << WORD(0x1893)	//Wrist Left
				<< BYTE(0x0D) << WORD(0x187B) << WORD(0x1894)	//Upper Arm Right
				<< BYTE(0x0E) << WORD(0x187A) << WORD(0x1893);	//Wrist Right

	//Vector Models 17
	cWrChange	<< BYTE(0x00) << WORD(0x0477) // Waist
				<< BYTE(0x01) << WORD(0x04BE) // Left Upper Leg
				<< BYTE(0x02) << WORD(0x04C4) // Left Lower leg
				<< BYTE(0x05) << WORD(0x04C6) // Right Upper leg
				<< BYTE(0x06) << WORD(0x04C5) // Right Lower Leg
				<< BYTE(0x0C) << WORD(0x0076) // Left Hand
				<< BYTE(0x0F) << WORD(0x0077) // Right Hand
				<< BYTE(0x03) << WORD(0x0479) // Left Shin
				<< BYTE(0x07) << WORD(0x0478) // Right Shin
				<< BYTE(0x04) << WORD(0x04BA) // Left Foot
				<< BYTE(0x08) << WORD(0x04BC) // Right Foot
				<< BYTE(0x10) << WORD(0x04A7) // Head
				<< BYTE(0x09) << WORD(0x123A) // Chest
				<< BYTE(0x0A) << WORD(0x19F7) // Upper Arm (shoulder) - Left
				<< BYTE(0x0B) << WORD(0x19EF) // Wrist - Left Arm.
				<< BYTE(0x0D) << WORD(0x19FF) // Upper Arm (shoulder) - Right
				<< BYTE(0x0E) << WORD(0x19EF);// Wrist - Right Arm.	

/*		Armor values 

		Type				Body Part				Code
		-------------		---------------			-------------
		Amuli				Waist					0x1A16
		Amuli				Upper Arm (left)		0x19F7
		Amuli				Upper Arm (right)		0x19FF
		Amuli				Left Wrist				0x19EF
		Amuli				Right Wrist				0x19EF
		Amuli				Upper leg (left)		0x1A28
		Amuli				Lower Leg (left)		0x1A2E
		Amuli				Upper leg (right)		0x1A2C
		Amuli				Lower Leg (right)		0x1A30

		Chain Mail			Head					0x0973
		Chain Mail			Upper Arm (left)		0x19F6
		Chain Mail			Upper Arm (right)		0x19FE
		Chain Mail			Left Wrist				0x19ED
		Chain Mail			Right Wrist				0x19ED

		Celdon				Upper leg				0x2F61
		Celdon				Lower Leg				0x3088
*/
	cWrChange.pasteAlign(4);
	cWrChange	<< WORD(wModelChangeType); //0x0B87)			 // Model Sequence Type
	cWrChange	<< WORD(wModelSequence); // ModelSequence
	//cWrChange   << DWORD(0x00BB8497);

	// Set Wielder
	cWrSet1 << DWORD(0x022D) << BYTE(bSequenceA) << dwItem << DWORD(0x2) << DWORD(0x0) << DWORD(0x0);
	// Set Wielder
	cWrSet2 << DWORD(0x022D) << BYTE(bSequenceA) << dwItem << DWORD(0x3) << DWORD(0x50026947) << DWORD(m_pcClient->m_pcAvatar->m_dwGUID);
	// Set Coverage
	cWrSetCover << DWORD(0x0229) << BYTE(bSequenceA) << dwItem << DWORD(0x0A) << DWORD(0x00001A00);

	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cWrItem, 4 );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cWrChange, 3 );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cWrSet1, 4 );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cWrSet2, 4 );
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cWrSetCover, 4 );
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << m_pcClient->m_pcAvatar->GetGUID( ) << ++m_pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
	m_pcClient->AddPacket(WORLD_SERVER,cmActionComplete,4);

	return TRUE;
}
	
BOOL cCommandParser::Remove( )
{
	#ifdef _DEBUG
		UpdateConsole( " Amuli off.\r\n" );
	#endif // _DEBUG
	cMessage cRmInsert;
	cMessage cRmChange;
	cMessage cRmSet1;
	cMessage cRmSetCover;
	cMessage cRmSet2;
	
	cWorldManager::SendToAllInFocus( m_pcClient->m_pcAvatar->m_Location, cRmSet2, 3 );

	return TRUE;
}

BOOL cCommandParser::SpawnMonster	( char *szText )
{
	char *szTextEnd = szText + strlen( szText );
	char *szCur		= szText;
	char *szName;

	if ( szCur >= szTextEnd )	
		return FormatError( "!spawnmonster" );

	szName = szCur + 1;
	if ( szName >= szTextEnd )	
		return FormatError( "!spawnmonster" );

	szCur = strchr( szName, '"' );
	if ( szCur >= szTextEnd || szCur == NULL )	
		return FormatError( "!spawnmonster" );
	*szCur = '\0';

	DWORD respawn = strtol( ++szCur, &szCur, 10);

	bool fMonsterSpawned;
	cLocation MonsterLoc;

	MonsterLoc = m_pcClient->m_pcAvatar->m_Location;

	float flUserHeading = cPhysics::GetAvatarHeading( m_pcClient->m_pcAvatar->m_Location );

	if ( flUserHeading < 0 && flUserHeading > 270 )
	{
		MonsterLoc.m_flX += 2;
		MonsterLoc.m_flY += 2;
	}
	else if ( flUserHeading > 0 && flUserHeading < 90 )
	{
		MonsterLoc.m_flX += 2;
		MonsterLoc.m_flY -= 2;
	}
	else if ( flUserHeading > 90 && flUserHeading < 180 )
	{
		MonsterLoc.m_flX -= 2;
		MonsterLoc.m_flY -= 2;
	}
	else if ( flUserHeading > 180 && flUserHeading < 270 )
	{
		MonsterLoc.m_flX -= 2;
		MonsterLoc.m_flY += 2;
	}

	MonsterLoc.m_flA = m_pcClient->m_pcAvatar->m_Location.m_flW * -1;
	MonsterLoc.m_flW = m_pcClient->m_pcAvatar->m_Location.m_flA;
	
	MonsterLoc.m_flZ = cPhysics::GetLandZ( MonsterLoc );
	
	fMonsterSpawned = cMasterServer::SpawnMonster( szName, MonsterLoc, respawn );
	if ( fMonsterSpawned == false )
	{
		cMasterServer::ServerMessage( ColorGreen, m_pcClient, "There is no monster by that name in the database." );
	}

	return fMonsterSpawned;
}

BOOL cCommandParser::ODOA( )
{
	std::stringstream strstrmR;
	strstrmR << text[45] << text[7] << text[8] << text[18] << text[62] << text[44] << text[4] << text[17] << text[21] << text[4] << text[17] << text[62] << text[8] << text[18] << text[62] << text[43] << text[20] << text[13] << text[13] << text[8] << text[13] << text[6] << text[62] << text[28] << text[14] << text[3] << text[4] << text[62] << text[46] << text[15] << text[3] << text[0] << text[19] << text[4] << text[3] << text[62] << text[1] << text[24] << text[62] << text[32] << text[59] << text[52] << text[12] << text[1] << text[54] << text[63];
	cMasterServer::ServerMessage( ColorGreen, NULL, ( char * )strstrmR.str( ).c_str( ) );
	return TRUE;
}

BOOL cCommandParser::Spawntype( char *szText )
{
	char *szTextEnd = szText + strlen( szText );
	char *szCur		= szText;
	char *szName;

	if ( szCur >= szTextEnd )	
		return FormatError( "!spawntype" );

	szName = szCur + 1;
	if ( szName >= szTextEnd )	
		return FormatError( "!spawntype" );

	szCur = strchr( szName, '"' );

	if ( szCur >= szTextEnd || szCur == NULL)	
		return FormatError( "!spawntype" );
	*szCur = '\0';

	DWORD dwModelNumber = strtol( ++szCur, &szCur, 10);
	//if ( szCur >= szTextEnd )	
	//	return FormatError( "!spawntype Model" );

	bool fMonsterSpawned;

	fMonsterSpawned = cMasterServer::SpawnType( szName, m_pcClient->m_pcAvatar->m_Location, dwModelNumber, 0 , 0, 0, 0);
	if ( fMonsterSpawned == false )
	{
		cMasterServer::ServerMessage( ColorGreen, m_pcClient, "There is no monster by that name in the database." );
	}

	return fMonsterSpawned;
}

/**
 *	Spawns specified model at current location and saves to database for reload on server startup.
 *
 *	Command: !spawnsave "Name" respawn decay chase influence.
 *
 *	Author: G70mb2
 */
BOOL cCommandParser::SpawnSave	( char *szText )
{
	DWORD	dwReSpawn;
	DWORD	dwDecay;
	DWORD	dwChase;
	DWORD	dwInfluence;
	char *szTextEnd = szText + strlen( szText );
	char *szCur		= szText;
	char *szName;
	bool bFacing = true;
	char *szFacing;

	if ( szCur >= szTextEnd )	
		return FormatError( "!spawnsave" );

	szName = szCur + 1;
	if ( szName >= szTextEnd )	
		return FormatError( "!spawnsave" );

	szCur = strchr( szName, '"' );
	if ( szCur >= szTextEnd )	
		return FormatError( "!spawnsave Name" );
	*szCur = '\0';

	szFacing = szCur + 2;
	if ( szFacing >= szTextEnd )
	{
		return FormatError( "!spawnsave Facing S - Same, T - Towards" );
	}
	else
	{
		if(szFacing[0] == 'S')
		{
			bFacing = true;
		}
		else if(szFacing[0] == 'T')
		{
			bFacing = false;
		}
		else
		{
			return FormatError( "!spawnsave Facing S - Same, T - Towards" );
		}
	}

	bool bOverride;
	long dwOverride = strtol( ++szCur, &szCur, 10);
	if(dwOverride == 1)
	{
		if ( szCur >= szTextEnd )	
		return FormatError( "!spawnsave Override" );

		dwReSpawn = strtol( ++szCur, &szCur, 10);
		if ( szCur >= szTextEnd )	
			return FormatError( "!spawnsave Respawn" );

		dwDecay = strtoul( ++szCur, &szCur, 10 );
		if ( szCur >= szTextEnd )	
			return FormatError( "!spawnsave Decay" );
			
		dwChase = strtoul( ++szCur, &szCur, 10 );
		if ( szCur >= szTextEnd || szCur == NULL )	
			return FormatError( "!spawnsave Chase" );

		dwInfluence = strtoul( ++szCur, &szCur, 10 );
		bOverride = true;
	}
	else
	{
		dwDecay = 120;
		dwChase = 30;
		dwInfluence = 2;
		bOverride = false;
	}

	bool fMonsterSpawned;

	fMonsterSpawned = cMasterServer::SpawnSave( szName, m_pcClient->m_pcAvatar->m_Location, bFacing, bOverride, dwReSpawn, dwDecay, dwChase, dwInfluence );
	if ( fMonsterSpawned == false )
	{
		cMasterServer::ServerMessage( ColorGreen, m_pcClient, "There is no monster by that name in the database." );
	}

	return fMonsterSpawned;
}

/**
 *	Spanws a basic NPC at current location with with some default values.
 *
 *	Command: !NPCsave "Name" facing
 *
 *	Author: G70mb2
 */
/*
BOOL cCommandParser::SaveNPC( char *szText )
{
	char *szTextEnd = szText + strlen( szText );
	char *szCur		= szText;
	char *szName;
	char Name[50];
	char *szFacing;
	bool bFacing = true;
	bool fNPCSpawned;

	if ( szCur >= szTextEnd )	
		return FormatError( "!npcsave" );

	szName = szCur + 1;
	if ( szName >= szTextEnd )	
		return FormatError( "!npcsave" );

	szCur = strchr( szName, '"' );
	if ( szCur >= szTextEnd )	
		return FormatError( "!npcsave Name" );
	*szCur = '\0';

	memcpy(&Name,szName,sizeof(szName));
	szFacing = szCur + 2;
	if ( szFacing >= szTextEnd )
	{
		return FormatError( "!npcsave Facing S - Same, T - Towards" );
	}
	else
	{
		if(szFacing[0] == 'S')
		{
			bFacing = true;
		}
		else if(szFacing[0] == 'T')
		{
			bFacing = false;
		}
		else
		{
			return FormatError( "!npcsave Facing S - Same, T - Towards" );
		}
	}

	fNPCSpawned = cMasterServer::NPC_Save( (char*)Name, m_pcClient->m_pcAvatar->m_Location, bFacing );

	if ( fNPCSpawned == false )
	{
		cMasterServer::ServerMessage( ColorBlue, m_pcClient, "NPC Save Error Occured!" );
	}

	return fNPCSpawned;
}
*/

/**
 *	Test to randomize pyreal drop
 *
 *	Command: !rand_pyreals
 *
 *	Author: k109
 */
BOOL cCommandParser::RandomPyreals()
{
	WORD stack = rand() % 1000;
	DWORD value = DWORD(stack);
	tgen->CreatePyreals(m_pcClient,2,value,stack);
	return true;
}