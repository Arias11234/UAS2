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
 *	@file Database.cpp
 *	Handles the general operations relating to the database.
 *
 *	Includes the functionality to create accounts, create, delete, and load avatars,
 *	and to initalize the Global Unique Identifier (GUID) values.
 */

#include <winsock2.h>

#include "DatFile.h"
#include "Database.h"
#include "WorldManager.h"
#include "MasterServer.h"

#define MAXBUFLEN   MAX_PATH+32

HENV	cDatabase::m_hEnv	=	0;
HDBC	cDatabase::m_hDBC	=	0;
HSTMT	cDatabase::m_hStmt	=	0;
int     cDatabase::wMaxModel =  0;
char	cDatabase::szModelName[1000][5];
char	cDatabase::szModelNumber[1000][3];
char	cDatabase::szDBIP[16];
char	cDatabase::szDBNAME[20];
char	cDatabase::szDBUSER[20];
char	cDatabase::szDBPASSWORD[20];
int		cDatabase::intDBType;

/**
 *	Loads the proper database driver based upon the selected database type.
 */
void cDatabase::Load( )
{
	char			szDirBuff[MAX_PATH+1];
	SQLCHAR			ConnStrIn[MAXBUFLEN];
	SQLCHAR			ConnStrOut[MAXBUFLEN];
	SQLSMALLINT		cbConnStrOut = 0;

	int index = GetCurrentDirectory(MAX_PATH, szDirBuff);

	if(szDirBuff[index] == '\\')
		szDirBuff[index] = '\n';

	//sprintf((char*)ConnStrIn, "FILEDSN=%s\\UAS_DB;", szDirBuff);
	switch(intDBType)
	{
	case 1:
		{
			sprintf((char*)ConnStrIn, "Driver={Microsoft Access Driver (*.mdb)};DBQ=%s;", cWorldManager::g_szAccessFile);//\\UASv1.2.mdb;", szDirBuff );
			break;
		}
	case 2:
		{
			sprintf((char*)ConnStrIn, "Driver={SQL Server};Server=%s;UserID=%s;Password=%s;Database=%s;", szDBIP,szDBUSER,szDBPASSWORD,szDBNAME);
			break;
		}
	case 3:
		{
//			sprintf((char*)ConnStrIn, "Driver={MySQL ODBC 3.51 Driver};Server=%s;User=%s;Password=%s;Database=%s;OPTION=3;", szDBIP,szDBUSER,szDBPASSWORD,szDBNAME);
			sprintf((char*)ConnStrIn, "Driver={MySQL ODBC 5.1 Driver};Server=%s;User=%s;Password=%s;Database=%s;OPTION=3;", szDBIP,szDBUSER,szDBPASSWORD,szDBNAME);
			break;
		}

	default:
		{
			sprintf((char*)ConnStrIn, "Driver={MySQL ODBC 5.1 Driver};Server=%s;User=%s;Password=%s;Database=%s;OPTION=3;", szDBIP,szDBUSER,szDBPASSWORD,szDBNAME);
			break;
		}	
	}
		//
	RETCODE retcode;

	retcode = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, (SQLHDBC FAR *)&m_hDBC);								CHECKRETURN(1, SQL_HANDLE_DBC, m_hDBC, 1)
	retcode = SQLDriverConnect(m_hDBC, g_hWndMain, ConnStrIn, SQL_NTS, ConnStrOut, MAXBUFLEN, &cbConnStrOut, SQL_DRIVER_COMPLETE_REQUIRED );	CHECKRETURN(1, SQL_HANDLE_DBC, m_hDBC, 1)
	retcode = SQLAllocHandle( SQL_HANDLE_STMT, m_hDBC, &m_hStmt );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, 1)
}

void cDatabase::Unload( )
{
	RETCODE retcode;

	retcode = SQLFreeHandle( SQL_HANDLE_STMT, m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )

	retcode = SQLDisconnect( m_hDBC );						CHECKRETURN( 1, SQL_HANDLE_DBC, m_hDBC, 1 )

	retcode = SQLFreeHandle( SQL_HANDLE_DBC, m_hDBC );		CHECKRETURN( 1, SQL_HANDLE_DBC, m_hDBC, 1 )
}

int cDatabase::InitializeSQLEnvironment( )
{
	RETCODE retcode;
	retcode = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv );								CHECKRETURN( 1, SQL_HANDLE_ENV, m_hEnv, 1 )
	retcode = SQLSetEnvAttr( m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER );	CHECKRETURN( 1, SQL_HANDLE_ENV, m_hEnv, 1 )
	return 0;
}

int cDatabase::FreeSQLEnvironment( )
{
	RETCODE retcode = SQLFreeHandle( SQL_HANDLE_ENV, m_hEnv );	CHECKRETURN( 1, SQL_HANDLE_ENV, m_hEnv, 1 )
	return 0;
}

void cDatabase::SetupDB( int DBType, char* DBIP, char* DBNAME, char* DBUSER, char* DBPASSWORD )
{
	memcpy(&szDBIP,DBIP,sizeof(szDBIP));
	memcpy(&szDBNAME,DBNAME,sizeof(szDBNAME));
	memcpy(&szDBUSER,DBUSER,sizeof(szDBUSER));
	memcpy(&szDBPASSWORD,DBPASSWORD,sizeof(szDBPASSWORD));
	intDBType			= DBType;
}

void cDatabase::GetError( SQLSMALLINT type, SQLHANDLE *hptr )
{
	SQLCHAR			State[5];
	SQLINTEGER		NativeError;
	SQLCHAR			ErrorText[500];
	*ErrorText = 0;
	SQLGetDiagRec( type, *hptr, 1, State, &NativeError, ErrorText, 500, NULL );
	if(*ErrorText!=0)
		UpdateConsole( "      <SQL> %s\r\n", ErrorText );
}

/**
 *	Initializes the starting avatar GUID value.
 *
 *	As GUIDs must be unique and persistent, saved avatars are consulted for their GUID values.
 *	Database tables consulted:  avatar.
 */
void cDatabase::InitializeAvatarGUIDs( DWORD &dwGUIDCycle_Avatar )
{
	char szCommand[100];

	RETCODE retcode;

	sprintf( szCommand, "SELECT MAX(AvatarGUID) FROM avatar;" );

	retcode = SQLPrepare( m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_ULONG, &dwGUIDCycle_Avatar, sizeof( DWORD ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFetch( m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		dwGUIDCycle_Avatar = 0x50000000;

	retcode = SQLCloseCursor( m_hStmt );															CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );													CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )	
}

/**
 *	Initializes the starting item GUID value.
 *
 *	As GUIDs must be unique and persistent, saved items are consulted for their GUID values.
 *	Database tables consulted:  items_instance_inventory and items_instance_ground.
 */
void cDatabase::InitializeObjectGUIDs( DWORD &dwGUIDCycle_Object )
{

	DWORD dwTempGUID = 0x80670000;

	char szCommand[100];
	RETCODE retcode;

	sprintf( szCommand, "SELECT MAX(GUID) FROM items_instance_inventory;" );

	retcode = SQLPrepare( m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_ULONG, &dwTempGUID, sizeof( DWORD ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFetch( m_hStmt );
	retcode = SQLCloseCursor( m_hStmt );															CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );
	
	dwGUIDCycle_Object = dwTempGUID;

	sprintf( szCommand, "SELECT MAX(GUID) FROM items_instance_ground;" );

	retcode = SQLPrepare( m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_ULONG, &dwTempGUID, sizeof( DWORD ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFetch( m_hStmt );
	retcode = SQLCloseCursor( m_hStmt );															CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );
		
	if( dwTempGUID > dwGUIDCycle_Object )
		dwGUIDCycle_Object = dwTempGUID;
}

/**
 *	Initializes the starting allegiance ID value.
 */
void cDatabase::InitializeAllegIDs( DWORD &dwIDAllegiance )
{

	DWORD dwTempGUID = 0x0L;

	char szCommand[100];
	RETCODE retcode;

	sprintf( szCommand, "SELECT MAX(AllegianceID) FROM allegiance;" );

	retcode = SQLPrepare( m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_ULONG, &dwTempGUID, sizeof( DWORD ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFetch( m_hStmt );
	retcode = SQLCloseCursor( m_hStmt );															CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );

	if( dwTempGUID > dwIDAllegiance )
		dwIDAllegiance = dwTempGUID;
}

/**
 *	Verifies the acount login information.
 */
BOOL cDatabase::VerifyAccount( char* szUserName, char* szPassword, DWORD &dwAccountID, BOOL fCreate )
{
	BOOL fNoRecords	= FALSE;

	char Command[200];
	char szDBPassword[45];

	RETCODE retcode;

	sprintf( Command, "SELECT ID, Password FROM account WHERE UserName='%s';", szUserName );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)Command, SQL_NTS );							CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );															CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_ULONG, &dwAccountID, sizeof( DWORD ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 2, SQL_C_CHAR, szDBPassword, sizeof( szDBPassword ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFetch( m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )

	if ( retcode == SQL_NO_DATA )
		fNoRecords = TRUE;
	
	retcode = SQLCloseCursor( m_hStmt );														CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );												CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )

	if( fNoRecords )
	{
		if( fCreate )
			if( CreateAccount( szUserName, szPassword, dwAccountID ) )
				return TRUE;
	}
	else if( lstrcmpi( szPassword, szDBPassword ) == 0 )
		return TRUE;
	
	return FALSE;
}

/**
 *	Creates a new account.  A given account may create several characters on a given server.
 */
BOOL cDatabase::CreateAccount( char* szUserName, char* szPassword, DWORD &dwAccountID )
{
	UpdateConsole( " <SQL> Creating account ...\r\n" );
						
	BOOL fVerify = TRUE;
	char Command[200];

	RETCODE retcode;

	sprintf( Command, "INSERT INTO Account (UserName, Password) VALUES ('%s', '%s');", szUserName, szPassword );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)Command, SQL_NTS );		CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )	
	
	if( retcode == SQL_ERROR )
		fVerify = FALSE;
	
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )
	
	if( fVerify )
		return VerifyAccount( szUserName, szPassword, dwAccountID, FALSE );
	else
		return FALSE;
}

/**
 *	Loads the list of avatars for a given account.
 *	Used to send the client a list of its current characters.
 */
void cDatabase::LoadAvatarList( DWORD dwAccountID, std::vector< cAvatarList > &AvatarList )
{
	AvatarList.clear( );

	char	szCommand[200];
	char	szName[75];
	BYTE	bAccessLevel;

	DWORD	dwGUID;
	DWORD	dwOwnerID;
	RETCODE	retcode;
	
	sprintf( szCommand, "SELECT AvatarGUID, OwnerID, Name, AccessLevel FROM avatar WHERE OwnerID=%d;", dwAccountID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS);					CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );														CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_ULONG, &dwGUID, sizeof( DWORD ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 2, SQL_C_ULONG, &dwOwnerID, sizeof( DWORD ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 3, SQL_C_CHAR, szName, sizeof( szName ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 4, SQL_C_USHORT, &bAccessLevel, sizeof( BYTE ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	
	cAvatarList cAL;
	
	for ( int i = 0; SQLFetch( m_hStmt ) == SQL_SUCCESS; ++i )
	{
		cAL.m_dwGUID	= dwGUID;
		cAL.m_dwOwnerID	= dwOwnerID;
		cAL.m_strName.assign( ReturnNamePrefix( bAccessLevel ) );
		cAL.m_strName.append( szName );
		AvatarList.push_back( cAL );
	}

	retcode = SQLCloseCursor( m_hStmt );												CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );										CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
}

/**
 *	Creates the database information for a newly created avatar.
 *
 *	The following tables are populated:
 *	avatar, avatar_skills, avatar_clothing_palettes, avatar_location, avatar_spells.
 */
BOOL cDatabase::CreateAvatar( DWORD dwAccountID, CreateCharacterMessage &ccm, DWORD &dwNewAvatarGUID )
{
	char szCommand[2048];

	RETCODE retcode;

	BOOL fAlreadyExists = TRUE;

	sprintf( szCommand, "SELECT ID FROM avatar WHERE Name='%s';", ccm.szName );
	
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFetch(m_hStmt );											CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	
	if( retcode == SQL_NO_DATA )
		fAlreadyExists = FALSE;
	
	retcode = SQLCloseCursor( m_hStmt );									CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )

	if( !fAlreadyExists )
	{
		dwNewAvatarGUID = cWorldManager::NewGUID_Avatar();						
																			
		//sprintf( szCommand, "INSERT INTO Avatar (AvatarGUID, OwnerID, Name, wModelNum, Race, Sex, SkinShade, HairColor, HairShade, HairStyle, EyeColor, Forehead, Nose, Chin, Class, Strength, StrengthBase, Endurance, EnduranceBase, Coordination, CoordinationBase, Quickness, QuicknessBase, Focus, FocusBase, Self, SelfBase, Birth) VALUES (%d, %d, '%s', 1, %d, %d, %f, %d, %f, %d, %d, %d, %d, %d, %d, 0, %d, 0, %d, 0, %d, 0, %d, 0, %d, 0, %d, %lu);"
		//					, dwNewAvatarGUID, dwAccountID, ccm.szName, ccm.dwRace, ccm.dwSex, ccm.dblSkinShade, ccm.dwHairColor, ccm.dblHairShade, ccm.dwHairStyle, ccm.dwEyeColor, ccm.dwForeheadTexture, ccm.dwNoseTexture, ccm.dwChinTexture, ccm.dwProfession, ccm.dwStrength, ccm.dwEndurance, ccm.dwCoordination, ccm.dwQuickness, ccm.dwFocus, ccm.dwSelf, time(NULL) );
		sprintf( szCommand, "INSERT INTO Avatar (AvatarGUID, OwnerID, Name, wModelNum, Race, Sex, SkinShade, HairColor, HairShade, HairStyle, EyeColor, Forehead, Nose, Chin, Class, Strength, StrengthBase, Endurance, EnduranceBase, Coordination, CoordinationBase, Quickness, QuicknessBase, Focus, FocusBase, Self, SelfBase) VALUES (%d, %d, '%s', 1, %d, %d, %f, %d, %f, %d, %d, %d, %d, %d, %d, 0, %d, 0, %d, 0, %d, 0, %d, 0, %d, 0, %d);"
							, dwNewAvatarGUID, dwAccountID, ccm.szName, ccm.dwRace, ccm.dwSex, ccm.dblSkinShade, ccm.dwHairColor, ccm.dblHairShade, ccm.dwHairStyle, ccm.dwEyeColor, ccm.dwForeheadTexture, ccm.dwNoseTexture, ccm.dwChinTexture, ccm.dwProfession, ccm.dwStrength, ccm.dwEndurance, ccm.dwCoordination, ccm.dwQuickness, ccm.dwFocus, ccm.dwSelf );
		
		retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
		retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
		retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

		std::ostringstream ss;
		ss << "INSERT INTO avatar_skills (AvatarGUID, AxeStatus, BowStatus, CrossbowStatus, DaggerStatus, MaceStatus, MeleeDefenseStatus, MissileDefenseStatus, SpearStatus, StaffStatus, SwordStatus, ThrownWeaponsStatus, UnarmedCombatStatus, ArcaneLoreStatus, MagicDefenseStatus, ManaConversionStatus, AppraiseItemStatus, AssessPersonStatus, DeceptionStatus, HealingStatus, JumpStatus, LockpickStatus, RunStatus, AssessCreatureStatus, AppraiseWeaponStatus, AppraiseArmorStatus, AppraiseMagicItemStatus, CreatureEnchantmentStatus, ItemEnchantmentStatus, LifeMagicStatus, WarMagicStatus,  LeadershipStatus,  LoyaltyStatus, FletchingStatus, AlchemyStatus, CookingStatus) VALUES (" << dwNewAvatarGUID;
		
		for( int i=1; i<0x28; i++ )
		{
			//Cubem0j0:  These are the Unknown skills
			if( (i==0x8) || (i==0x11) || (i==0x19) || (i==0x1A) )
				continue;
			else
				ss << ", " << ccm.dwSkillStatus[i];

		}
		ss << ");";

		retcode = SQLPrepare( m_hStmt, (unsigned char *)ss.str().c_str(), SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
		retcode = SQLExecute( m_hStmt );												CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
		retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );									CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

		/** Determine New Avatar Clothing **/
		/*
		0x00000001 - blue-green
		0x00000002 - teal
		0x00000003 - violet
		0x00000004 - light brown
		0x00000005 - blue
		0x00000006 - dark brown
		0x00000007 - dark green
		0x00000008 - light green
		0x00000009 - white/gray/black
		0x0000000A - turquoise
		0x0000000B - magenta
		0x0000000C - purple-blue
		0x0000000D - purple
		0x0000000E - red-brown
		0x0000000F - dark purple
		0x00000010 - brown
		0x00000011 - yellow
		0x00000012 - gold
		*/

		DWORD	apparelTypes[4] = { ccm.dwHatType,ccm.dwShirtType,ccm.dwPantsType,ccm.dwShoeType };
		DWORD	apparelColors[4] = { ccm.dwHatColor,ccm.dwShirtColor,ccm.dwPantsColor,ccm.dwShoeColor };
		double	apparelShades[4] = { ccm.dblHatShade,ccm.dblShirtShade,ccm.dblPantsShade,ccm.dblShoeShade };
		DWORD	apparelModelNums[4];

		switch (ccm.dwRace)
		{
			case CHARDATA_RACE_ALUVIAN:
			{
				/* Aluvian
				Headgear:
					0xFFFFFFFF - None
					0x00000000 - Cloth Cap
					0x00000001 - Cloth Cowl
				Shirt:
					0x00000000 - Shirt
					0x00000001 - Tunic
					0x00000002 - Baggy Shirt
					0x00000003 - Baggy Tunic
					0x00000004 - Doublet
					0x00000005 - Smock
				Trousers:
					0x00000000 - Wide Breeches
					0x00000001 - Trousers
					0x00000002 - Pants
				Footwear:
					0x00000000 - Leather Boots
					0x00000001 - Shoes
				*/
				switch (ccm.dwHatType)
				{
					case 0xFFFFFFFF: break;
					case 0x00000000: apparelModelNums[0] = 1973; break;
					case 0x00000001: apparelModelNums[0] = 1974; break;
				}
				switch (ccm.dwShirtType)
				{
					case 0x00000000: apparelModelNums[1] = 1972; break;
					case 0x00000001: apparelModelNums[1] = 1979; break;
					case 0x00000002: apparelModelNums[1] = 1934; break;
					case 0x00000003: apparelModelNums[1] = 1980; break;
					case 0x00000004: apparelModelNums[1] = 1935; break;
					case 0x00000005: apparelModelNums[1] = 1970; break;
				}
				switch (ccm.dwPantsType)
				{
					case 0x00000000: apparelModelNums[2] = 1971; break;
					case 0x00000001: apparelModelNums[2] = 1978; break;
					case 0x00000002: apparelModelNums[2] = 1977; break;
				}
				switch (ccm.dwShoeType)
				{
					case 0x00000000: apparelModelNums[3] = 1976; break;
					case 0x00000001: apparelModelNums[3] = 1975; break;
				}
				break;
			}
			case CHARDATA_RACE_GHARU:
			{
				/* Gharu'ndim
				Headgear:
					0xFFFFFFFF - None
					0x00000000 - Fez
					0x00000001 - Qafiya
					0x00000002 - Turban
				Shirt:
					0x00000000 - Baggy Tunic
					0x00000001 - Jerkin
					0x00000002 - Loose Shirt
					0x00000003 - Puffy Shirt
					0x00000004 - Puffy Tunic
					0x00000005 - Smock
				Trousers:
					0x00000000 - Baggy Breeches
					0x00000001 - Baggy Pants
					0x00000002 - Pantaloons
				Footwear:
					0x00000000 - Leather Boots
					0x00000001 - Slippers
				*/
				switch (ccm.dwHatType)
				{
					case 0xFFFFFFFF: break;
					case 0x00000000: apparelModelNums[0] = 1941; break;
					case 0x00000001: apparelModelNums[0] = 1936; break;
					case 0x00000002: apparelModelNums[0] = 1944; break;
				}
				switch (ccm.dwShirtType)
				{
					case 0x00000000: apparelModelNums[1] = 1980; break;
					case 0x00000001: apparelModelNums[1] = 1937; break;
					case 0x00000002: apparelModelNums[1] = 1942; break;
					case 0x00000003: apparelModelNums[1] = 1945; break;
					case 0x00000004: apparelModelNums[1] = 1946; break;
					case 0x00000005: apparelModelNums[1] = 1970; break;
				}
				switch (ccm.dwPantsType)
				{
					case 0x00000000: apparelModelNums[2] = 1940; break;
					case 0x00000001: apparelModelNums[2] = 1938; break;
					case 0x00000002: apparelModelNums[2] = 1943; break;
				}
				switch (ccm.dwShoeType)
				{
					case 0x00000000: apparelModelNums[3] = 1976; break;
					case 0x00000001: apparelModelNums[3] = 1939; break;
				}
				break;
			}
			case CHARDATA_RACE_SHO:
			{
				/* Sho
				Headgear:
					0xFFFFFFFF - None
					0x00000000 - Cloth Cap
					0x00000001 - Kasa
				Shirt:
					0x00000000 - Baggy Shirt
					0x00000001 - Doublet
					0x00000002 - Flared Shirt
					0x00000003 - Flared Tunic
					0x00000004 - Loose Shirt
					0x00000005 - Loose Tunic			
				Trousers:
					0x00000000 - Flared Pants
					0x00000001 - Loose Breeches
					0x00000002 - Loose Pants				
				Footwear:
					0x00000000 - Leather Boots
					0x00000001 - Shoes
				*/
				switch (ccm.dwHatType)
				{
					case 0xFFFFFFFF: break;
					case 0x00000000: apparelModelNums[0] = 1973; break;
					case 0x00000001: apparelModelNums[0] = 1948; break;
				}
				switch (ccm.dwShirtType)
				{
					case 0x00000000: apparelModelNums[1] = 1934; break;
					case 0x00000001: apparelModelNums[1] = 1935; break;
					case 0x00000002: apparelModelNums[1] = 1950; break;
					case 0x00000003: apparelModelNums[1] = 1952; break;
					case 0x00000004: apparelModelNums[1] = 1942; break;
					case 0x00000005: apparelModelNums[1] = 1953; break;
				}
				switch (ccm.dwPantsType)
				{
					case 0x00000000: apparelModelNums[2] = 1947; break;
					case 0x00000001: apparelModelNums[2] = 1949; break;
					case 0x00000002: apparelModelNums[2] = 1951; break;
				}
				switch (ccm.dwShoeType)
				{
					case 0x00000000: apparelModelNums[3] = 1976; break;
					case 0x00000001: apparelModelNums[3] = 1975; break;
				}
				break;
			}
		}

		cItemModels *pcModel;
		cClothes *aClothes;
		for (int apparelIndex = 0; apparelIndex < 4; ++apparelIndex)
		{
			if (apparelTypes[apparelIndex] != 0xFFFFFFFF)	//No clothing = 0xFFFFFFFF (-1)
			{

				pcModel = cItemModels::FindModel(apparelModelNums[apparelIndex]);
				aClothes = new cClothes(cWorldManager::NewGUID_Object(),dwNewAvatarGUID,apparelModelNums[apparelIndex],1.0,TRUE,0x0000,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);

				if (ccm.dwSex == 0) //female
					cPortalDat::LoadItemModel(aClothes, 0x0200004E, apparelColors[apparelIndex], apparelShades[apparelIndex]);
				else				//male
					cPortalDat::LoadItemModel(aClothes, 0x02000001, apparelColors[apparelIndex], apparelShades[apparelIndex]);
				aClothes->m_dwOwnerID = dwNewAvatarGUID;
				aClothes->m_fEquipped = 2;
				aClothes->m_intColor = apparelColors[apparelIndex];
				
				char	data[512];
				sprintf (data, "%d %d %d %d %s",apparelModelNums[apparelIndex],1,aClothes->m_intColor,aClothes->m_wIcon,"0");

				char	szCommand[512];
				RETCODE	retcode;
				sprintf( szCommand, "INSERT INTO items_instance_inventory (GUID, OwnerGUID, Equipped, Data) VALUES(%lu, %lu, %d, '%s');",aClothes->GetGUID(),dwNewAvatarGUID,2,data);

				retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );

				for (int paletteIndex = 0; paletteIndex < aClothes->m_bWearPaletteChange; ++paletteIndex)
				{
					sprintf( szCommand, "INSERT INTO avatar_clothing_palettes ( ObjectGUID,VectorCount,NewPalette,Offset,Length ) VALUES (%lu,%d,'%04x','%02x','%02x');",aClothes->GetGUID(),paletteIndex+1,aClothes->m_WearVectorPal[paletteIndex].m_wNewPalette,aClothes->m_WearVectorPal[paletteIndex].m_ucOffset,aClothes->m_WearVectorPal[paletteIndex].m_ucLength );
					retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );
				}
			}
		}
		
		//***********************Starting Locations***********************
		//*****AC:DM*****
		//	0 - Holtburg South:		Landblock: A9B00014	X: 56.065	Y: 94.537	Z: 64.929	W: -0.985335	X: 0	Y: 0	Z: -0.170631
		//  0 - Holtburg South:		Landblock: A9B00015	X: 63.333	Y: 104.547	Z: 64.574	W: -0.860791	X: 0	Y: 0	Z: -0.508959
		//	1 - Holtburg West:		Landblock: A5B4002A	X: 125.1	Y: 30.476	Z: 53.467	W: 0.983105		X: 0	Y: 0	Z: -0.183042
		//	1 - Holtburg West:		Landblock: A5B40032	X: 144.833	Y: 33.441	Z: 52.005	W: -0.95051		X: 0	Y: 0	Z: -0.310693
		//	2 - Shoushi Southeast:	Landblock: DE51001D	X: 76.396	Y: 103.326	Z: 16.005	W: -0.990424	X: 0	Y: 0	Z: -0.138058
		//	2 - Shoushi Southeast:	Landblock: DE51001D	X: 87.589	Y: 101.218	Z: 15.144	W: -0.955041	X: 0	Y: 0	Z: -0.296475
		//	3 - Shoushi West:		Landblock: D6550023	X: 97.4		Y: 63.8		Z: 52.005	W: 0.878817		X: 0	Y: 0	Z: -0.477159
		//	3 - Shoushi West:		Landblock: D6550023	X: 107.8	Y: 61.7		Z: 52.005	W: 0.999048		X: 0	Y: 0	Z: -0.76041
		//	4 - Yaraq North:		Landblock: 7D680012	X: 52.204	Y: 37.99	Z: 16.343	W: 0.335353		X: 0	Y: 0	Z: -0.942093
		//	4 - Yaraq North:		Landblock: 7D680012	X: 60.712	Y: 37.987	Z: 16.343	W: 0.215281		X: 0	Y: 0	Z: -0.976552
		//	5 - Yaraq East:			Landblock: 8164000D	X: 32.599	Y: 103.123	Z: 30.734	W: -0.924113	X: 0	Y: 0	Z: -0.382119
		//	5 - Yaraq East:			Landblock: 8164000D	X: 37.984	Y: 111.845	Z: 31.166	W: -0.846806	X: 0	Y: 0	Z: -0.531901

		//*****AC:ToD*****
		//	0 - Holtburg:			Landblock: 860201AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	0 - Holtburg:			Landblock: 860301AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	0 - Holtburg:			Landblock: 860401AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	0 - Holtburg:			Landblock: 870201AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	0 - Holtburg:			Landblock: 870301AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7

		//	1 - Shoushi:			Landblock: 7F0301AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	1 - Shoushi:			Landblock: 7F0401AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	1 - Shoushi:			Landblock: 800201AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	1 - Shoushi:			Landblock: 800301AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	1 - Shoushi:			Landblock: 800401AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7

		//	2 - Yaraq:				Landblock: 8C0401AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	2 - Yaraq:				Landblock: 8D0201AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	2 - Yaraq:				Landblock: 8D0301AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	2 - Yaraq:				Landblock: 8D0401AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	2 - Yaraq:				Landblock: 8E0201AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7

		//	3 - Sanamar:			Landblock: 720201AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	3 - Sanamar:			Landblock: 720301AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	3 - Sanamar:			Landblock: 720401AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	3 - Sanamar:			Landblock: 730201AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7
		//	3 - Sanamar:			Landblock: 730301AD	X: 41451E4F	Y: C1E3DB23	Z: 3BA3D70A	W: 3EAD8A54	X: 00000000	Y: 00000000	Z: 3F70DBA7

		int startLoc = 0 + int(2 * rand() / (RAND_MAX + 1.0)); //Each starting region has two distinct starting locations

		switch(ccm.dwStartingPlace)
		{
			case 0:		//Holtburg South
				{
					if (startLoc == 0)
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'A9B00014','4260428F','42BD12F2','4281DBA6','BF7C3EEB','00000000','80000000','BE2EB9D8','A9B00007', '41ACB08A', '431291AA', '42680000', 'BF532C0D', '00000000', '00000000', 'BF10B6F8');", dwNewAvatarGUID );
					else
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'A9B00015','427D54FE','42D11810','428125E3','BF5C5CCA','00000000','80000000','BF024B20','A9B00007', '41ACB08A', '431291AA', '42680000', 'BF532C0D', '00000000', '00000000', 'BF10B6F8');", dwNewAvatarGUID );
					retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );
					break;
				}
			case 1:		//Holtburg West
				{
					if (startLoc == 0)
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'A5B4002A','42FA3333','41F3CED9','4255DE35','3F7BACC7','00000000','00000000','BE3B6F5C','A5B4003D', '432E1DB2', '42CE2560', '42380000', '3F6CBA12', '00000000', '00000000', 'BEC2E625');", dwNewAvatarGUID );
					else
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'A5B40032','4310D53F','4205C396','4250051F','BF7354A5','00000000','00000000','BE9F1324','A5B4003D', '432E1DB2', '42CE2560', '42380000', '3F6CBA12', '00000000', '00000000', 'BEC2E625');", dwNewAvatarGUID );
					retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );				
					break;
				}
			case 2:		//Shoushi Southeast
				{
					if (startLoc == 0)
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'DE51001D','4298CAC1','42CEA6E9','41800A3D','BF7D8C6F','00000000','00000000','BE0D5F23','DE510016', '425A6666', '42F03333', '41800000', '42B1999A', '00000000', '00000000', '00000000');", dwNewAvatarGUID );
					else
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'DE51001D','42AF2D91','42CA6F9E','41724DD3','BF747D8B','00000000','00000000','BE97CB91','DE510016', '425A6666', '42F03333', '41800000', '42B1999A', '00000000', '00000000', '00000000');", dwNewAvatarGUID );
					retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );					
					break;
				}
			case 3:		//Shoushi West
				{
					if (startLoc == 0)
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'D6550023','42C2CCCD','427F3333','4250051F','3F60FA28','00000000','00000000','BEF44E28','D655003C', '43286666', '42B0CCCD', '42300000', '4330B333', '00000000', '00000000', '00000000');", dwNewAvatarGUID );
					else
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'D6550023','42D7999A','4276CCCD','4250051F','3F7FC1A0','00000000','00000000','BF42AA3E','D655003C', '43286666', '42B0CCCD', '42300000', '4330B333', '00000000', '00000000', '00000000');", dwNewAvatarGUID );
					retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );					
					break;
				}
			case 4:		//Yaraq North
				{
					if (startLoc == 0)
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'7D680012','4250D0E5','4217F5C3','4182BE77','3EABB365','00000000','00000000','BF712CFA','7D680021', '42D834BC', '4162A027', '41200000', '3F37C2B8', '00000000', '00000000', '3F323C63');", dwNewAvatarGUID );
					else
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'7D680012','4272D917','4217F2B0','4182BE77','3E5C7298','00000000','00000000','BF79FF53','7D680021', '42D834BC', '4162A027', '41200000', '3F37C2B8', '00000000', '00000000', '3F323C63');", dwNewAvatarGUID );
					retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );					
					break;
				}
			case 5:		//Yaraq East
				{
					if (startLoc == 0)
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'8164000D','42026560','42CE3EFA','41F5DF3B','BF6C92AE','00000000','00000000','BEC3A516','81640007', '4199BDA5', '43140625', '41C00000', '3F1D5AE2', '00000000', '00000000', '3F49EDDF');", dwNewAvatarGUID );
					else
						sprintf( szCommand, "INSERT INTO avatar_location ( AvatarGUID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z,LS_Landblock,LS_Position_X,LS_Position_Y,LS_Position_Z,LS_Orientation_W,LS_Orientation_X,LS_Orientation_Y,LS_Orientation_Z) VALUES (%lu,'8164000D','4217EF9E','42DFB0A4','41F953F8','BF58C84F','00000000','00000000','BF082AAD','81640007', '4199BDA5', '43140625', '41C00000', '3F1D5AE2', '00000000', '00000000', '3F49EDDF');", dwNewAvatarGUID );
					retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
					retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );
					break;
				}	
		}

		//k109: Staring Spells based on skills

		int c_spell_id[8];
		int i_spell_id[8];
		int l_spell_id[8];
		int w_spell_id[8];

		int t_spell_count = 0;
		int s_spell_count = 0;

		//Creature Trained -- Invuln Other/Self I, Focus Self I
		if (ccm.dwSkillStatus[0x1F] == 2)
		{
			c_spell_id[0] = 17;
			c_spell_id[1] = 18;
			c_spell_id[2] = 1421;
			
			for(int s = 0; s < 3; s++)
			{
				t_spell_count += 1;
				AddSpell(dwNewAvatarGUID,t_spell_count,c_spell_id[s]);
			}
		}
				
		//Creature Specd -- Invuln Other/Self I, Focus Self I, Willpower Self I
		if (ccm.dwSkillStatus[0x1F] == 3)
		{
			c_spell_id[0] = 17;
			c_spell_id[1] = 18;
			c_spell_id[2] = 1421;
			c_spell_id[3] = 1445;
			
			for(int s = 0; s < 4; s++)
			{
				s_spell_count += 1;
				AddSpell(dwNewAvatarGUID,s_spell_count,c_spell_id[s]);
			}
		}
		
		//Item Trained -- Blood Drinker I, Impen I, Bludg Bane I, Swift Killer I
		if (ccm.dwSkillStatus[0x20] == 2)
		{
			i_spell_id[0] = 35;
			i_spell_id[1] = 49;
			i_spell_id[2] = 51;
			i_spell_id[3] = 1511;

			for(int s = 0; s < 4; s++)
			{
				t_spell_count += 1;
				AddSpell(dwNewAvatarGUID,t_spell_count,i_spell_id[s]);
			}

		}
		//Item Specd -- Blood Drinker I, Impen I, Bludg Bane I, Swift Killer I, Blade Bane I, Defender I
		if (ccm.dwSkillStatus[0x20] == 3)
		{
			i_spell_id[0] = 35;
			i_spell_id[1] = 49;
			i_spell_id[2] = 51;
			i_spell_id[3] = 1511;
			i_spell_id[4] = 37;
			i_spell_id[5] = 1599;
			
			for(int s = 0; s < 6; s++)
			{
				s_spell_count += 1;
				AddSpell(dwNewAvatarGUID,s_spell_count,i_spell_id[s]);
			}

		}
			
		//Life Trained -- Armor Other/Self I, Heal Other/Self I, Imperil I
		if (ccm.dwSkillStatus[0x21] == 2)
		{
			l_spell_id[0] = 23;
			l_spell_id[1] = 24;
			l_spell_id[2] = 5;
			l_spell_id[3] = 6;
			l_spell_id[4] = 25;
			
			for(int s = 0; s < 5; s++)
			{
				t_spell_count += 1;
				AddSpell(dwNewAvatarGUID,t_spell_count,l_spell_id[s]);
			}
		}
		//Life Specd -- Armor Other/Self I, Heal Other/Self I, Imperil I, Drain Health Other I, Harm Other I
		if (ccm.dwSkillStatus[0x21] == 3)
		{
			l_spell_id[0] = 23;
			l_spell_id[1] = 24;
			l_spell_id[2] = 5;
			l_spell_id[3] = 6;
			l_spell_id[4] = 25;
			l_spell_id[5] = 1237;
			l_spell_id[6] = 7;

			for(int s = 0; s < 7; s++)
			{
				s_spell_count += 1;
				AddSpell(dwNewAvatarGUID,s_spell_count,l_spell_id[s]);
			}
		}
		
		//War Trained -- Flame Bolt, Force Bolt, Frost Bolt, Shock Wave
		if (ccm.dwSkillStatus[0x22] == 2)
		{
			w_spell_id[0] = 27;
			w_spell_id[1] = 28;
			w_spell_id[2] = 86;
			w_spell_id[3] = 64;

			for(int s = 0; s < 4; s++)
			{
				t_spell_count += 1;
				AddSpell(dwNewAvatarGUID,t_spell_count,w_spell_id[s]);
			}
			
		}
		//War Specd
		if (ccm.dwSkillStatus[0x22] == 3)
		{
			w_spell_id[0] = 27;
			w_spell_id[1] = 28;
			w_spell_id[2] = 58;
			w_spell_id[3] = 64;
			w_spell_id[4] = 75;
			w_spell_id[5] = 86;
			w_spell_id[6] = 92;
			
			for(int s = 0; s < 7; s++)
			{
				s_spell_count += 1;
				AddSpell(dwNewAvatarGUID,s_spell_count,w_spell_id[s]);
			}
		}
		
		DWORD total_spells = s_spell_count += t_spell_count;

		sprintf( szCommand, "INSERT INTO Avatar_Spells ( OwnerGUID,dwSpellCount ) VALUES (%d,%d);         ", dwNewAvatarGUID, total_spells);
		retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
		retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
		retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );
			//	UpdateConsole("Chin: %#x, Fore: %#x, Nose: %#x Skintone: %x04 \r\n", ccm.dwChinTexture, ccm.dwForeheadTexture, ccm.dwNoseTexture, ccm.dwSkinPalette);
		
		//Casting items
		if(total_spells > 0)
		{
			AddCasterItems(dwNewAvatarGUID);
			AddFoci(dwNewAvatarGUID,ccm);
		}

		//Add Starting Equipment
			AddGeneralItems(dwNewAvatarGUID);
		
		/* Racial Equipment */
		if (ccm.dwRace == CHARDATA_RACE_SHO)
			AddRaceItemsSho(dwNewAvatarGUID);

		if (ccm.dwRace == CHARDATA_RACE_GHARU)
			AddRaceItemsGharundim(dwNewAvatarGUID);

		if (ccm.dwRace == CHARDATA_RACE_ALUVIAN)
			AddRaceItemsAluvian(dwNewAvatarGUID);

		/* Profession Equipment */
		AddProfessionItems(dwNewAvatarGUID,ccm);

			

	}
	return !fAlreadyExists;
}

/**
 *	Deletes the database information on a particular avatar.
 *
 *	Entries are deleted from the following tables:
 *	avatar, avatar_skills, avatar_location, avatar_spells, avatar_vector_spellbook,
 *	avatar_vector_spelltabs, avatar_vector_equipped, avatar_clothing_palettes, 
 *	items_instance_inventory, avatar_comp_quests.
 */
void cDatabase::DeleteAvatar( DWORD dwAvatarGUID )
{
	char szCommand[256];
	// Remove record from the avatar table
	sprintf( szCommand, "DELETE FROM avatar WHERE AvatarGUID=%lu;         ", dwAvatarGUID );

	RETCODE retcode;
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Remove record from the avatar_skills table
	sprintf( szCommand, "DELETE FROM avatar_skills WHERE AvatarGUID=%lu;          ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Remove record from the avatar_location table
	sprintf( szCommand, "DELETE FROM avatar_location WHERE AvatarGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Remove records from the avatar_spells table
	sprintf( szCommand, "DELETE FROM avatar_spells WHERE OwnerGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Remove record from the avatar_vector_spellbook table
	sprintf( szCommand, "DELETE FROM avatar_vector_spellbook WHERE OwnerGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Remove record from the avatar_vector_spelltabs table
	sprintf( szCommand, "DELETE FROM avatar_vector_spelltabs WHERE OwnerGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Remove record from the avatar_vector_equipped table
	sprintf( szCommand, "DELETE FROM avatar_vector_equipped WHERE OwnerGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Iterate through items in the avatar's inventory
	DWORD dwTempGUID;
	std::list< DWORD >	lstInventory;

	sprintf( szCommand, "SELECT GUID FROM items_instance_inventory WHERE OwnerGUID=%lu;		", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_ULONG, &dwTempGUID, sizeof( DWORD ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		lstInventory.push_back( dwTempGUID );
	}
	retcode = SQLCloseCursor( m_hStmt );															CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );
	for ( std::list<DWORD>::iterator list_iter = lstInventory.begin(); list_iter != lstInventory.end(); ++list_iter )
	{
		// Remove the unique palettes for items in the avatar's inventory
		sprintf( szCommand, "DELETE FROM avatar_clothing_palettes WHERE ObjectGUID=%lu;       ", *list_iter );

		retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
		retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
		retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )
	}
	lstInventory.clear();

	// Remove the avatar's inventory
	sprintf( szCommand, "DELETE FROM items_instance_inventory WHERE OwnerGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	//Remove quest records
	sprintf( szCommand, "DELETE FROM avatar_comp_quests WHERE AvatarGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

/*
	// Remove record from the npc_vector_textures table
	sprintf( szCommand, "DELETE FROM npcs_vector_textures WHERE OwnerGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Remove record from the npc_vector_palettes table
	sprintf( szCommand, "DELETE FROM npcs_vector_palettes WHERE OwnerGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )

	// Remove record from the npc_vector_models table
	sprintf( szCommand, "DELETE FROM npcs_vector_models WHERE OwnerGUID=%lu;       ", dwAvatarGUID );

	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, 1 )
*/
}

/**
 *	Loads the database information on a particular avatar during login.
 *
 *	Database tables consulted:  avatar, avatar_skills, avatar_location,
 *	houses_covenants, avatar_vector_spellbook, avatar_vector_spelltab, 
 *	items_instance_inventory.
 */
void cDatabase::LoadAvatar( cAvatar* pcAvatar )
{
	UINT	dwGUID = pcAvatar->GetGUID( );
	DWORD	dwAccountID;
	char	szOptionsFlag[9];

	char	dwLandblock[9];
	char	dwPosX[9];
	char	dwPosY[9];
	char	dwPosZ[9];
	char	dwOrientW[9];
	char	dwOrientX[9];
	char	dwOrientY[9];
	char	dwOrientZ[9];
	char	dwLSLandblock[9];
	char	dwLSPosX[9];
	char	dwLSPosY[9];
	char	dwLSPosZ[9];
	char	dwLSOrientW[9];
	char	dwLSOrientX[9];
	char	dwLSOrientY[9];
	char	dwLSOrientZ[9];
	char	dwCovLandblock[9];
	char	dwCovPosX[9];
	char	dwCovPosY[9];
	char	dwCovPosZ[9];
	char	dwCovOrientW[9];
	char	dwCovOrientX[9];
	char	dwCovOrientY[9];
	char	dwCovOrientZ[9];

	char	szCommand[512];
	DWORD	dwAvatarsGUID;
	WORD	dwHouseID;
	DWORD	dwSpellID;
	float	flCharge;
	WORD	wSpellType;
	DWORD	dwUnkA,dwUnkB;
	DWORD	dwSpell;
	WORD	intTab;
	WORD	intCount;
	
	int i;
	RETCODE			retcode;

	sprintf( szCommand, "SELECT * FROM avatar WHERE AvatarGUID=%lu;", dwGUID );
	
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLExecute( m_hStmt );										CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	
	int iCol = 4;
	
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_CHAR, pcAvatar->m_strCharacterName, sizeof( pcAvatar->m_strCharacterName ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_bAccessLevel, sizeof( BYTE ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wModelNum, sizeof( WORD), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_flAScale, sizeof( pcAvatar->m_flScale ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)

	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wRace, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wGender, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_DOUBLE, &pcAvatar->m_dblSkinShade, sizeof( &pcAvatar->m_dblSkinShade ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wHairColor, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_DOUBLE, &pcAvatar->m_dblHairShade, sizeof( &pcAvatar->m_dblHairShade ), NULL );CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wHairStyle, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wEyeColor, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)

	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wHead, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wNose, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wChin, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)

	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwTotalXP, sizeof( DWORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwUnassignedXP, sizeof( DWORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_cStats.m_dwLevel, sizeof( DWORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_bTotalSkillCredits, sizeof( BYTE ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_bSkillCredits, sizeof( BYTE ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wClass, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)

	for(i = 0; i < 6; i++ )
	{
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_cStats.m_lpcAttributes[i].m_dwIncrement, sizeof( DWORD ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_cStats.m_lpcAttributes[i].m_dwCurrent, sizeof( DWORD ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_cStats.m_lpcAttributes[i].m_dwXP, sizeof( DWORD ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	}

	for(i = 0; i < 3; i++ )
	{
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_cStats.m_lpcVitals[i].m_dwIncreases, sizeof( DWORD ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_cStats.m_lpcVitals[i].m_dwXP, sizeof( DWORD ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	}

	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_fIsPK, sizeof( DWORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwAllegianceID, sizeof( DWORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwNumFollowers, sizeof( DWORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwNumDeaths, sizeof( DWORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_CHAR, &szOptionsFlag, sizeof( szOptionsFlag ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
//	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwBirth, sizeof( DWORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
//	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwNumLogins, sizeof( DWORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	
	retcode = SQLFetch( m_hStmt );																		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	
	sscanf(szOptionsFlag,"%08x",&pcAvatar->m_dwOptions);

	pcAvatar->m_dwBirth = 0;
	pcAvatar->m_dwNumLogins = 0;

	retcode = SQLCloseCursor( m_hStmt );																CHECKRETURN(0, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );														CHECKRETURN(0, SQL_HANDLE_STMT, m_hStmt, NULL)

	cPortalDat::LoadStartingInfo( pcAvatar );

	sprintf( szCommand, "SELECT * FROM avatar_skills WHERE AvatarGUID=%lu;", dwGUID );
	
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLExecute( m_hStmt );																	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	
	iCol = 3;
	for(i = 0; i < 40; i++ )
	{
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_cStats.m_lpcSkills[i].m_wID, sizeof( WORD ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_cStats.m_lpcSkills[i].m_wStatus, sizeof( WORD ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_cStats.m_lpcSkills[i].m_dwIncreases, sizeof( DWORD ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	
		pcAvatar->CalcSkill(i);	// sets m_cStats.m_lpcSkills[i].m_dwTotal
	}
	
	retcode = SQLFetch( m_hStmt );																		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLCloseCursor( m_hStmt );																CHECKRETURN(0, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );														CHECKRETURN(0, SQL_HANDLE_STMT, m_hStmt, NULL)

	pcAvatar->m_strName.assign( ReturnNamePrefix( pcAvatar->m_bAccessLevel ) );
	pcAvatar->m_strName.append( pcAvatar->m_strCharacterName );


	for(i = 0; i < 40; i++ )
	{
		pcAvatar->m_cStats.m_lpcSkills[i].m_dwXP = 0;

		if (pcAvatar->m_cStats.m_lpcSkills[i].m_wStatus == 3) //specialized
			sprintf( szCommand, "SELECT spec_exp FROM exp_table WHERE ID = %d;", pcAvatar->m_cStats.m_lpcSkills[i].m_dwIncreases );
		else if (pcAvatar->m_cStats.m_lpcSkills[i].m_wStatus == 2) //trained
			sprintf( szCommand, "SELECT trained_exp FROM exp_table WHERE ID = %d;", pcAvatar->m_cStats.m_lpcSkills[i].m_dwIncreases );

		retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLExecute( m_hStmt );
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &pcAvatar->m_cStats.m_lpcSkills[i].m_dwXP, sizeof( DWORD ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLFetch( m_hStmt );																		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLCloseCursor( m_hStmt );																CHECKRETURN(0, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );														CHECKRETURN(0, SQL_HANDLE_STMT, m_hStmt, NULL)
	}

	//Vitals
	pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent = pcAvatar->m_cStats.m_lpcVitals[0].m_dwCurrent = floor((double)(pcAvatar->m_cStats.m_lpcAttributes[1].m_dwCurrent + pcAvatar->m_cStats.m_lpcAttributes[1].m_dwIncrement) / 2 + .5) + pcAvatar->m_cStats.m_lpcVitals[0].m_dwIncreases;
	pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent = pcAvatar->m_cStats.m_lpcVitals[1].m_dwCurrent = (pcAvatar->m_cStats.m_lpcAttributes[1].m_dwCurrent + pcAvatar->m_cStats.m_lpcAttributes[1].m_dwIncrement) + pcAvatar->m_cStats.m_lpcVitals[1].m_dwIncreases;
	pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent = pcAvatar->m_cStats.m_lpcVitals[2].m_dwCurrent = (pcAvatar->m_cStats.m_lpcAttributes[5].m_dwCurrent + pcAvatar->m_cStats.m_lpcAttributes[5].m_dwIncrement) + pcAvatar->m_cStats.m_lpcVitals[2].m_dwIncreases;

	//Location
	sprintf( szCommand, "SELECT * FROM avatar_location WHERE AvatarGUID=%lu;", dwGUID );
	
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLExecute( m_hStmt );
	
	iCol = 3;
	// Avatar location
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_Location.m_flX, sizeof( pcAvatar->m_Location.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_Location.m_flY, sizeof( pcAvatar->m_Location.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_Location.m_flZ, sizeof( pcAvatar->m_Location.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_Location.m_flA, sizeof( pcAvatar->m_Location.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_Location.m_flB, sizeof( pcAvatar->m_Location.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_Location.m_flC, sizeof( pcAvatar->m_Location.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_Location.m_flW, sizeof( pcAvatar->m_Location.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// Lifestone location
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLSLandblock, sizeof( dwLSLandblock ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_LSLoc.m_flX, sizeof( pcAvatar->m_LSLoc.m_flX ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_LSLoc.m_flY, sizeof( pcAvatar->m_LSLoc.m_flY ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_LSLoc.m_flZ, sizeof( pcAvatar->m_LSLoc.m_flZ ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_LSLoc.m_flA, sizeof( pcAvatar->m_LSLoc.m_flA ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_LSLoc.m_flB, sizeof( pcAvatar->m_LSLoc.m_flB ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_LSLoc.m_flC, sizeof( pcAvatar->m_LSLoc.m_flC ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_LSLoc.m_flW, sizeof( pcAvatar->m_LSLoc.m_flW ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLSPosX, sizeof( dwLSPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLSPosY, sizeof( dwLSPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLSPosZ, sizeof( dwLSPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLSOrientW, sizeof( dwLSOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLSOrientX, sizeof( dwLSOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLSOrientY, sizeof( dwLSOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLSOrientZ, sizeof( dwLSOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLFetch( m_hStmt );		CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)

	sscanf(dwLandblock,"%08x",&pcAvatar->m_Location.m_dwLandBlock);
	sscanf(dwPosX,"%08x",&pcAvatar->m_Location.m_flX);
	sscanf(dwPosY,"%08x",&pcAvatar->m_Location.m_flY);
	sscanf(dwPosZ,"%08x",&pcAvatar->m_Location.m_flZ);
	sscanf(dwOrientW,"%08x",&pcAvatar->m_Location.m_flA);
	sscanf(dwOrientX,"%08x",&pcAvatar->m_Location.m_flB);
	sscanf(dwOrientY,"%08x",&pcAvatar->m_Location.m_flC);
	sscanf(dwOrientZ,"%08x",&pcAvatar->m_Location.m_flW);
	sscanf(dwLSLandblock,"%08x",&pcAvatar->m_LSLoc.m_dwLandBlock);
	sscanf(dwLSPosX,"%08x",&pcAvatar->m_LSLoc.m_flX);
	sscanf(dwLSPosY,"%08x",&pcAvatar->m_LSLoc.m_flY);
	sscanf(dwLSPosZ,"%08x",&pcAvatar->m_LSLoc.m_flZ);
	sscanf(dwLSOrientW,"%08x",&pcAvatar->m_LSLoc.m_flA);
	sscanf(dwLSOrientX,"%08x",&pcAvatar->m_LSLoc.m_flB);
	sscanf(dwLSOrientY,"%08x",&pcAvatar->m_LSLoc.m_flC);
	sscanf(dwLSOrientZ,"%08x",&pcAvatar->m_LSLoc.m_flW);

	// House (covenant crystal) location
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
	
	sprintf( szCommand, "SELECT * FROM avatar WHERE AvatarGUID = %lu;",dwGUID );									
	retcode = SQLPrepare( cDatabase::m_hStmt,(unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );

	retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwAccountID, sizeof( dwAccountID ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

	if (SQLFetch( cDatabase::m_hStmt ) != SQL_SUCCESS)
	{
		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
	} else {
		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

//		sprintf( szCommand, "SELECT COUNT(houses_covenants.ID) FROM {oj avatar LEFT OUTER JOIN houses_covenants ON avatar.AvatarGUID=houses_covenants.OwnerID} WHERE avatar.OwnerID = %d;",AccountID );

		bool owner = false;
		dwAvatarsGUID = NULL;

		sprintf( szCommand, "SELECT AvatarGUID FROM avatar WHERE OwnerID = %lu;",dwAccountID );
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwAvatarsGUID, sizeof( dwAvatarsGUID ), NULL );CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
		DWORD avatarArray[10];

		for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
		{
			avatarArray[i] = dwAvatarsGUID;
		}

		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		
		for ( i = 0; owner != true && i < 10; i++ )
		{
			sprintf( szCommand, "SELECT HouseID FROM houses_covenants WHERE OwnerID = %lu;",avatarArray[i] );
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			retcode = SQLExecute( cDatabase::m_hStmt );														CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwHouseID, sizeof( WORD ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

			if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
			{
				owner = true;
			}
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	
		}
	
		if( owner == true ) {
			//sprintf( szCommand, "SELECT houses_covenants.HouseID,houses_covenants.Landblock,houses_covenants.fl_X,houses_covenants.fl_Y,houses_covenants.fl_Z,houses_covenants.fl_heading,houses_covenants.Unknown_1,houses_covenants.Unknown_2,houses_covenants.Heading_2 FROM {oj avatar LEFT OUTER JOIN houses_covenants ON avatar.AvatarGUID=houses_covenants.OwnerID} WHERE avatar.OwnerID = %lu;",AccountID );		
			sprintf( szCommand, "SELECT HouseID,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z FROM houses_covenants WHERE HouseID = %d;", dwHouseID );
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			retcode = SQLExecute( cDatabase::m_hStmt );														CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			
			int iCol = 1;							
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wHouseID, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovLandblock, sizeof( dwCovLandblock ), NULL );										CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	/*	 	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_HRLoc.m_flX, sizeof( &pcAvatar->m_HRLoc.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_HRLoc.m_flY, sizeof( &pcAvatar->m_HRLoc.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_HRLoc.m_flZ, sizeof( &pcAvatar->m_HRLoc.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_HRLoc.m_flA, sizeof( &pcAvatar->m_HRLoc.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_HRLoc.m_flB, sizeof( &pcAvatar->m_HRLoc.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_HRLoc.m_flC, sizeof( &pcAvatar->m_HRLoc.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &pcAvatar->m_HRLoc.m_flW, sizeof( &pcAvatar->m_HRLoc.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	*/		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovPosX, sizeof( dwCovPosX ), NULL );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovPosY, sizeof( dwCovPosY ), NULL );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovPosZ, sizeof( dwCovPosZ ), NULL );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovOrientW, sizeof( dwCovOrientW ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovOrientX, sizeof( dwCovOrientX ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovOrientY, sizeof( dwCovOrientY ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovOrientZ, sizeof( dwCovOrientZ ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

			SQLFetch( cDatabase::m_hStmt );

			sscanf(dwCovLandblock,"%08x",&pcAvatar->m_HRLoc.m_dwLandBlock);
			sscanf(dwCovPosX,"%08x",&pcAvatar->m_HRLoc.m_flX);
			sscanf(dwCovPosY,"%08x",&pcAvatar->m_HRLoc.m_flY);
			sscanf(dwCovPosZ,"%08x",&pcAvatar->m_HRLoc.m_flZ);
			sscanf(dwCovOrientW,"%08x",&pcAvatar->m_HRLoc.m_flA);
			sscanf(dwCovOrientX,"%08x",&pcAvatar->m_HRLoc.m_flB);
			sscanf(dwCovOrientY,"%08x",&pcAvatar->m_HRLoc.m_flC);
			sscanf(dwCovOrientZ,"%08x",&pcAvatar->m_HRLoc.m_flW);

			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		}
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );		CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	}

	// Load the Primary Spell Info - Tells us the lengths of the Vectors to load
	sprintf( szCommand, "SELECT * FROM avatar_spells WHERE OwnerGUID='%lu';", dwGUID);	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLExecute( m_hStmt );													CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

	iCol = 3;
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &pcAvatar->m_wSpellCount , sizeof( WORD ), NULL );	
	retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwSpellUnknown , sizeof( DWORD ), NULL );
	 	
	 //k109:  This appears to be just the loading of the tabs. Spellcount only used in next section.

	for(i=0;i < 7; i++)
	{
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_SpellTabs[i].dwTabCount , sizeof( DWORD ), NULL );
	}
		
	retcode = SQLFetch( m_hStmt );					CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLCloseCursor( m_hStmt );			CHECKRETURN(0, SQL_HANDLE_STMT, m_hStmt, NULL)
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

	// Load Vector Spell Book into Array
	if(pcAvatar->m_wSpellCount != 0)
	{ // If Spell Count = 0 than nothing to load
		sprintf( szCommand, "SELECT * FROM avatar_vector_spellbook WHERE OwnerGUID=%lu;",pcAvatar->GetGUID());
		
		retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );			CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		retcode = SQLExecute( m_hStmt );												CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

		iCol = 4;
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &dwSpellID, sizeof( DWORD ), NULL );		
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_FLOAT, &flCharge, sizeof( flCharge ), NULL );	
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &wSpellType, sizeof( WORD), NULL );
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &dwUnkA, sizeof( DWORD ), NULL );
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &dwUnkB, sizeof( DWORD ), NULL );

		for ( int intSB = 0; SQLFetch( m_hStmt ) == SQL_SUCCESS && intSB < pcAvatar->m_wSpellCount; intSB++ )
		{
			pcAvatar->m_SpellBook[intSB].dwSpell_ID = dwSpellID;
			pcAvatar->m_SpellBook[intSB].flCharge = flCharge;
			pcAvatar->m_SpellBook[intSB].wSpellType= wSpellType;
			pcAvatar->m_SpellBook[intSB].dwUnknownA = dwUnkA;
			pcAvatar->m_SpellBook[intSB].dwUnknownB = dwUnkB;
		}

		retcode = SQLCloseCursor( m_hStmt );			CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)

		// Load Vector SpellTabs into Array

		sprintf( szCommand, "SELECT * FROM avatar_vector_spelltabs WHERE OwnerGUID=%lu;",pcAvatar->GetGUID());
		
		retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		retcode = SQLExecute( m_hStmt );											CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		iCol = 3;
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &intCount, sizeof( intCount ), NULL );
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_USHORT, &intTab, sizeof( intTab ), NULL );		
		retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &dwSpell, sizeof( dwSpell ), NULL );

		for ( int intST = 0; SQLFetch( m_hStmt ) == SQL_SUCCESS && intST < pcAvatar->m_wSpellCount; intST++ )
		{
			pcAvatar->m_SpellTabs[intTab].dwSpell_ID[intCount] = dwSpell;
		}

		retcode = SQLCloseCursor( m_hStmt );			CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
		retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );	CHECKRETURN(1, SQL_HANDLE_STMT, m_hStmt, NULL)
	}// End Spell If

	//////////////////// Start Inventory Stuff ////////////////
	// Load Vector Inventory Count

	sprintf( szCommand, "SELECT COUNT(ID) FROM items_instance_inventory WHERE OwnerGUID=%lu;",pcAvatar->GetGUID());
	
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
	retcode = SQLExecute( m_hStmt );
	if(SQLFetch( m_hStmt ) == SQL_SUCCESS)
	{
        retcode = SQLBindCol( m_hStmt, iCol++, SQL_C_ULONG, &pcAvatar->m_dwInventoryCount, sizeof( pcAvatar->m_dwInventoryCount ), NULL );
	}
	else
	{
		pcAvatar->m_dwInventoryCount = 0;
	}
	retcode = SQLCloseCursor( m_hStmt ); 
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );

	// Load Vector Inventory into Array
	if(pcAvatar->m_dwInventoryCount > 0)
	{ // Only do if there is actually something in the Inventory
		DWORD	ObjectGUID;
		
		char	data[512];
		DWORD	dwItemModelNumber;
		DWORD	dwItemAmount;
		WORD	wIcon;
//		int		intColor;
		char	spells[255];

		sprintf( szCommand, "SELECT GUID,Data FROM items_instance_inventory WHERE OwnerGUID = %lu;",pcAvatar->GetGUID() );
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			
		retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &ObjectGUID, sizeof( DWORD ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, 5, SQL_C_CHAR, &data, sizeof( data ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; i++ )
		{
			sscanf(data,"%d %d %d %s",&dwItemModelNumber,&dwItemAmount,&wIcon,&spells);
			cItemModels *pcModel = cItemModels::FindModel(dwItemModelNumber); //Find the model
			
			for ( int intINV = 0; SQLFetch( m_hStmt ) == SQL_SUCCESS; intINV++ )
			{
				pcAvatar->m_vInventory[intINV].dwObjectGUID = ObjectGUID;
				pcAvatar->m_vInventory[intINV].dwIsContainer = pcModel->m_isContainer;
			}
		}
		retcode = SQLCloseCursor( m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );		CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	}// End Inventory If
	//////////////////// End Inventory Stuff ////////////////
}

/**
 *	Populates a list with the information on the name and location of dungeons.
 */
BOOL cDatabase::LoadDungeonList( char strName, std::vector< cDungeonList > &DungeonList  )
{
	DungeonList.clear( );

	cDungeonList	cDL;
	char			szCommand[200];
	char			szName[75];
	INT				intID;
	char			dwLandblock[9];
	char			dwPosX[9];
	char			dwPosY[9];
	char			dwPosZ[9];
	char			dwOrientW[9];
//	char			dwOrientX[9];
//	char			dwOrientY[9];
//	char			dwOrientZ[9];
//	float			fPosX;
//	float			fPosY;
//	float			fPosZ;
//	float			fHeading;
	
	RETCODE	retcode;
	
	sprintf( szCommand, "SELECT ID, Name, Landblock, Position_X, Position_Y, Position_Z, Heading FROM dungeons WHERE Name=%d;", strName );

	int iCol = 1;
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS);								CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &intID, sizeof( INT ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szName, sizeof( szName ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fPosX, sizeof( &fPosX ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fPosY, sizeof( &fPosY ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fPosZ, sizeof( &fPosZ ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fHeading, sizeof( &fHeading ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	for ( int i = 0; SQLFetch( m_hStmt ) == SQL_SUCCESS; ++i ) {
		sscanf(dwLandblock,"%08x",&cDL.m_dwLandblock);
		sscanf(dwPosX,"%08x",&cDL.m_flLocX);
		sscanf(dwPosY,"%08x",&cDL.m_flLocX);
		sscanf(dwPosZ,"%08x",&cDL.m_flLocZ);
		sscanf(dwOrientW,"%08x",&cDL.m_flHeading);
//		sscanf(dwOrientX,"%08x",&cDL.m_flB);
//		sscanf(dwOrientY,"%08x",&cDL.m_flC);
//		sscanf(dwOrientZ,"%08x",&cDL.m_flW);
		
		cDL.m_intID = intID;
		cDL.m_strName = szName;
		DungeonList.push_back( cDL );
	}
	retcode = SQLCloseCursor( m_hStmt );												CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );										CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )

	if (SQL_SUCCESS) { return TRUE; }
	if (!SQL_SUCCESS) { return FALSE; }
}

void cDatabase::InitializeMaxModel( )
{
	char szCommand[150];
//	char szMaxModel[150];

	RETCODE retcode;

	sprintf( szCommand, "SELECT MAX(ModelNUM) FROM npcs_models;" );

	retcode = SQLPrepare( m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_ULONG, &wMaxModel, sizeof( wMaxModel ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFetch( m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		wMaxModel = 0;

	retcode = SQLCloseCursor( m_hStmt );			CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )	
	
//	sprintf( szMaxModel, " Max Model Number has been set to:  %d.\r\n",wMaxModel );
//	UpdateConsole((char *)szMaxModel);
//	UpdateConsole("\r\n");
}

void cDatabase::LoadModelList( )
{
	char szCommand[100];
//	char szModels[150];

	DWORD	dwModelCount;
	char szName[100];
	WORD wModelNum;

	RETCODE retcode;

	sprintf( szCommand, "SELECT COUNT(ID) FROM npcs_models;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwModelCount, sizeof( dwModelCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		dwModelCount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
//	sprintf( szModels, " Loading %d models ... ",dwModelCount );
//	UpdateConsole((char *)szModels);

	sprintf( szCommand, "SELECT ModelNum, ModelName FROM npcs_models ORDER BY ModelNum;" );

	retcode = SQLPrepare( m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 1, SQL_C_USHORT, &wModelNum, sizeof( wModelNum ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLBindCol( m_hStmt, 2, SQL_C_CHAR, &szName, sizeof( szName ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )

	for ( int i = 0; SQLFetch( m_hStmt ) == SQL_SUCCESS; ++i )
	{
		sprintf( szModelName[i],"%s",szName );
		sprintf( szModelNumber[i],"%d",wModelNum );
	}

	retcode = SQLCloseCursor( m_hStmt );			CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )
	retcode = SQLFreeStmt( m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, m_hStmt, NULL )	
}

/**
 *	Adds items to the avatar's inventory table
 */
void cDatabase::AddToInventoryDB(DWORD dwAvatarGUID, cObject* pcObj)
{
	char	data[512];
	sprintf (data, "%d %d %d %d %s",pcObj->m_dwModel,pcObj->m_wStack,pcObj->m_intColor,pcObj->m_wIcon,"1");

	char	szCommand[512];
	RETCODE	retcode;
	sprintf( szCommand, "INSERT INTO items_instance_inventory (GUID, OwnerGUID, Equipped, Data) VALUES(%lu, %lu, %d, '%s');",pcObj->m_dwGUID,dwAvatarGUID,0,data);

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );
}
/**
 *	Removes items from the avatar's inventory table
 */	
void cDatabase::RemoveFromInventoryDB(DWORD dwAvatarGUID, DWORD dwObjectGUID)
{
	char	szCommand[512];
	RETCODE	retcode;
	sprintf( szCommand, "DELETE FROM items_instance_inventory WHERE OwnerGUID = %lu AND GUID = %lu;",dwAvatarGUID,dwObjectGUID);

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );
}										

/**
 *	Adds spells to the avatars spell book.
 *	
 *	@param dwAvatarGUID = The Avatar GUID
 *	@param SpellCount 	= Total spell count for this Avatar
 *	@param SpellID 		= The ID of the spell to add
 *
 *	Author: k109
 */
void cDatabase::AddSpell (DWORD dwAvatarGUID, int SpellCount, int SpellID)
{
	char szCommand[500];
	RETCODE retcode;
	
	sprintf( szCommand, "INSERT INTO avatar_vector_spellbook (OwnerGUID, ItemCount, dwSpellID, flCharge, intSpellType, dwUnknownA, dwUnknownB) VALUES(%lu,%d,%d,1,0,0,0);",dwAvatarGUID,SpellCount,SpellID);
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );
}

/**
 *	Adds spells to the avatar's spell tab.
 *	
 *	dwAvatarGUID 	= The Avatar GUID
 *	SpellID 		= The ID of the spell to add
 *	SpellCount 		= Total spell count for this Avatar
 *	TabID			= ID of the Spell Tab
 *
 *	Author: k109
 */
void cDatabase::AddSpellToTab (DWORD dwAvatarGUID, int SpellID, int SpellCount, int TabID)
{
	char szCommand[500];
	RETCODE retcode;
	
	sprintf( szCommand, "INSERT INTO avatar_vector_spelltabs (OwnerGUID, ItemCount, Tab_Num, dwSpellID) VALUES(%d,%d,%d,%d);",dwAvatarGUID,SpellCount,TabID,SpellID);
	retcode = SQLPrepare( m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLExecute( m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, m_hStmt, 1 )
	retcode = SQLFreeStmt( m_hStmt, SQL_CLOSE );
}

void cDatabase::AddGeneralItems (DWORD dwAvatarGUID)
{
	/*
	pcModel = cItemModels::FindModel(2);
	cPyreals *StartingCash = new cPyreals(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,10000,25000);
	AddToInventoryDB(dwAvatarGUID, StartingCash);
	*/
	cItemModels *pcModel;
	pcModel = cItemModels::FindModel(1965);
	cObject *CallingStone = new cGems(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	AddToInventoryDB(dwAvatarGUID, CallingStone);
	
	pcModel = cItemModels::FindModel(1964);
	cObject *WelcomeLetter = new cBooks(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	AddToInventoryDB(dwAvatarGUID, WelcomeLetter);
}

void cDatabase::AddRaceItemsAluvian (DWORD dwAvatarGUID)
{
	cItemModels *pcModel;
	pcModel = cItemModels::FindModel(218);
	cObject *Bread = new cFood(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,5,pcModel->m_wBurden,2,5);
	AddToInventoryDB(dwAvatarGUID, Bread);

	pcModel = cItemModels::FindModel(1954);
	cObject *Dagger = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
		pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
	AddToInventoryDB(dwAvatarGUID, Dagger);
}

void cDatabase::AddRaceItemsGharundim (DWORD dwAvatarGUID)
{
	cItemModels *pcModel;
	pcModel = cItemModels::FindModel(218);
	cObject *Bread = new cFood(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,5,pcModel->m_wBurden,2,5);
	AddToInventoryDB(dwAvatarGUID, Bread);

	pcModel = cItemModels::FindModel(1961);
	cObject *Staff = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
		pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
	AddToInventoryDB(dwAvatarGUID, Staff);
}

void cDatabase::AddRaceItemsSho (DWORD dwAvatarGUID)
{
	cItemModels *pcModel;
	pcModel = cItemModels::FindModel(266);
	cObject *Apples = new cFood(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,5,pcModel->m_wBurden,2,5);
	AddToInventoryDB(dwAvatarGUID, Apples);

	pcModel = cItemModels::FindModel(1957);
	cObject *Cestus = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
		pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
	AddToInventoryDB(dwAvatarGUID, Cestus);
		
}

void cDatabase::AddCasterItems (DWORD dwAvatarGUID)
{
		cItemModels *pcModel;
		pcModel = cItemModels::FindModel(1966);
		cObject *StarterWand = new cWands(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
		AddToInventoryDB(dwAvatarGUID, StarterWand);

		pcModel = cItemModels::FindModel(321);	//Lead Scarabs
		cObject *Scarabs = new cSpellComps(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,5,pcModel->m_wBurden);
		Scarabs->m_wStack = 5;
		AddToInventoryDB(dwAvatarGUID, Scarabs);

		pcModel = cItemModels::FindModel(327);	//Prismatic tapers
		cObject *Tapers = new cSpellComps(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,5,pcModel->m_wBurden);
		Tapers->m_wStack = 25;
		AddToInventoryDB(dwAvatarGUID, Tapers);
}

void cDatabase::AddProfessionItems (DWORD dwAvatarGUID, CreateCharacterMessage &ccm)
{
	cItemModels *pcModel;
	if (ccm.dwSkillStatus[SKILL_HEALING] == 2 || ccm.dwSkillStatus[SKILL_HEALING] == 3)
	{
		pcModel = cItemModels::FindModel(290);
		cHealingCon *kit = new cHealingCon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wBurden,pcModel->m_dwValue,pcModel->m_wUses,pcModel->m_wUseLimit);
		AddToInventoryDB(dwAvatarGUID, kit);
	}

	if (ccm.dwSkillStatus[SKILL_LOCKPICK] == 2 || ccm.dwSkillStatus[SKILL_LOCKPICK] == 3)
	{
		pcModel = cItemModels::FindModel(188);
		cLockpicks* Picks = new cLockpicks(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wUses, pcModel->m_wUseLimit);
		AddToInventoryDB(dwAvatarGUID, Picks);
	}

	if (ccm.dwSkillStatus[SKILL_AXE] == 2 || ccm.dwSkillStatus[SKILL_AXE] == 3)
	{
		pcModel = cItemModels::FindModel(1959);
		cObject *Axe = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
			pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
		AddToInventoryDB(dwAvatarGUID, Axe);
	}

	if (ccm.dwSkillStatus[SKILL_BOW] == 2 || ccm.dwSkillStatus[SKILL_BOW] == 3)
	{
		pcModel = cItemModels::FindModel(1955);
		cObject *Shortbow = new cAmmo(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,1,1);
		AddToInventoryDB(dwAvatarGUID, Shortbow);

		pcModel = cItemModels::FindModel(147);
		cObject *Arrows = new cAmmo(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,250,250);
		Arrows->m_wStack = 250;
		AddToInventoryDB(dwAvatarGUID, Arrows);
	}

	if (ccm.dwSkillStatus[SKILL_CROSSBOW] == 2 || ccm.dwSkillStatus[SKILL_CROSSBOW] == 3)
	{
		pcModel = cItemModels::FindModel(1962);
		cObject *LightXbow = new cAmmo(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,1,1);
		AddToInventoryDB(dwAvatarGUID, LightXbow);

		pcModel = cItemModels::FindModel(145);
		cObject *Quarrel = new cAmmo(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,250,250);
		Quarrel->m_wStack = 250;
		AddToInventoryDB(dwAvatarGUID, Quarrel);
	}

	if (ccm.dwSkillStatus[SKILL_DAGGER] == 2 || ccm.dwSkillStatus[SKILL_DAGGER] == 3)
	{
		pcModel = cItemModels::FindModel(1954);
		cObject *Dagger = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
			pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
		AddToInventoryDB(dwAvatarGUID, Dagger);
	}

	if (ccm.dwSkillStatus[SKILL_MACE] == 2 || ccm.dwSkillStatus[SKILL_MACE] == 3)
	{
		pcModel = cItemModels::FindModel(1958);
		cObject *Mace = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
			pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
		AddToInventoryDB(dwAvatarGUID, Mace);
	}

	if (ccm.dwSkillStatus[SKILL_SPEAR] == 2 || ccm.dwSkillStatus[SKILL_SPEAR] == 3)
	{
		pcModel = cItemModels::FindModel(1960);
		cObject *Spear = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
			pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
		AddToInventoryDB(dwAvatarGUID, Spear);
	}

	if (ccm.dwSkillStatus[SKILL_STAFF] == 2 || ccm.dwSkillStatus[SKILL_STAFF] == 3)
	{
		pcModel = cItemModels::FindModel(1961);
		cObject *Staff = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
			pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
		AddToInventoryDB(dwAvatarGUID, Staff);
	}

	if (ccm.dwSkillStatus[SKILL_SWORD] == 2 || ccm.dwSkillStatus[SKILL_SWORD] == 3)
	{
		pcModel = cItemModels::FindModel(1956);
		cObject *Sword = new cWeapon(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,
			pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,3,25,0x0D,0x04,.75,1,.5,0);
		AddToInventoryDB(dwAvatarGUID, Sword);
	}

	if (ccm.dwSkillStatus[SKILL_THROWN_WEAPONS] == 2 || ccm.dwSkillStatus[SKILL_THROWN_WEAPONS] == 3)
	{
		pcModel = cItemModels::FindModel(1963);
		cObject *Atlatl = new cAmmo(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,1,1);
		AddToInventoryDB(dwAvatarGUID, Atlatl);

		pcModel = cItemModels::FindModel(155);
		cObject *Darts = new cAmmo(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,250,250);
		Darts->m_wStack = 250;
		AddToInventoryDB(dwAvatarGUID, Darts);
	}
}

void cDatabase::AddFoci(DWORD dwAvatarGUID, CreateCharacterMessage &ccm)
{
		cItemModels *pcModel;
		if (ccm.dwSkillStatus[SKILL_WAR_MAGIC] == 2 || ccm.dwSkillStatus[SKILL_WAR_MAGIC] == 3)
		{
			pcModel = cItemModels::FindModel(287);
			cObject *Foci = new cFoci(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
			AddToInventoryDB(dwAvatarGUID, Foci);
		}

		if (ccm.dwSkillStatus[SKILL_LIFE_MAGIC] == 2 || ccm.dwSkillStatus[SKILL_LIFE_MAGIC] == 3)
		{
			pcModel = cItemModels::FindModel(286);
			cObject *Foci = new cFoci(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
			AddToInventoryDB(dwAvatarGUID, Foci);
		}

		if (ccm.dwSkillStatus[SKILL_ITEM_ENCHANTMENT] == 2 || ccm.dwSkillStatus[SKILL_ITEM_ENCHANTMENT] == 3)
		{
			pcModel = cItemModels::FindModel(289);
			cObject *Foci = new cFoci(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
			AddToInventoryDB(dwAvatarGUID, Foci);
		}

		if (ccm.dwSkillStatus[SKILL_CREATURE_ENCHANTMENT] == 2 || ccm.dwSkillStatus[SKILL_CREATURE_ENCHANTMENT] == 3)
		{
			pcModel = cItemModels::FindModel(288);
			cObject *Foci = new cFoci(cWorldManager::NewGUID_Object(),dwAvatarGUID,pcModel->m_dwModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
			AddToInventoryDB(dwAvatarGUID, Foci);
		}
}

void cDatabase::SaveCharacterFlags(DWORD dwAvatarGUID, DWORD Flags)
{
	char szCommand[100];
	RETCODE retcode;
	sprintf( szCommand, "UPDATE avatar SET OptionsFlag = ('%08x') WHERE AvatarGUID = %lu;",Flags,dwAvatarGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}
/**
 *	Converts 32-bit hexadecimal values to floating point numbers.
 *	
 *	Expects hexadecimal input.  Returns a float value.
 */
float cDatabase::Hex2Float (DWORD hexValue)
{
	char	charArrayValue[9];
	float	floatValue;
	sscanf(charArrayValue,"%08x",&floatValue);

	return floatValue;
}
/**	
 *	Converts floating point numbers to 32-bit hexadecimal values.
 *	
 *	Expects float input.  Returns DWORD.
 */
DWORD cDatabase::Float2Hex (float floatValue)
{
	union 
	{
		float          f;
		unsigned char  b[sizeof(float)];
	} v = { floatValue };

	size_t	s;
//	char	c[8];

	// Big Endian
//	for ( s = 0; s < sizeof(v.b); ++s) { sprintf(c, "%02X", v.b[s]); }
	// Little Endian
//	for ( s = sizeof(v.b); s > 0; --s) { sprintf(c, "%02X", v.b[i-s]); }

	char	c1[2], c2[2], c3[2], c4[2];
	char	hexTemp[9];
	DWORD hexValue;

	s = sizeof(v.b);
	sprintf(c1, "%02X", v.b[s-1]);
	sprintf(c2, "%02X", v.b[s-2]);
	sprintf(c3, "%02X", v.b[s-3]);
	sprintf(c4, "%02X", v.b[s-4]);

	int i, j;
	i = j = 0;
	while(c1[i]) { hexTemp[i] = c1[i]; ++i;}
	j = 0;
	while(c2[j]) { hexTemp[i] = c2[j]; ++i; ++j; }
	j = 0;
	while(c3[j]) { hexTemp[i] = c3[j]; ++i; ++j; }
	j = 0;
	while(c4[j]) { hexTemp[i] = c4[j]; ++i; ++j; }
	
	hexTemp[i] = '\0';
	sscanf(hexTemp,"%08x",&hexValue);

	return hexValue;
}
