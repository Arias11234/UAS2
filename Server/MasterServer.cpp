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
 *	@file MasterServer.cpp
 *	Implements functionality for Master Server thread.
 *
 *	The Master Server controls the loading and unloading of world objects.
 *	It also implements basic commands.
 */

#include "Client.h"
#include "CommandParser.h"
#include "DatFile.h"
#include "Job.h"
#include "MasterServer.h"
#include "RecvPacket.h"
#include "WorldManager.h"
#include "Database.h"
#include "Status.h"
#include "cItemModels.h"
#include "cWObjectModels.h"
#include "TreasureGen.h"
#include "cNPCModels.h"
#include "cSpell.h"

HANDLE				cMasterServer::m_hThread			= 0;
HANDLE				cMasterServer::m_hExitEvent			= 0;
UINT_PTR			cMasterServer::m_uipTimer			= 0;
UINT_PTR			cMasterServer::m_uipStatusTimer		= 0;
cJobPool*			cMasterServer::m_pcJobPool			= 0;
int					cMasterServer::m_iLoadCount			= 0;
DWORD				cMasterServer::m_dwThreadID			= 0;
cLocation			cMasterServer::m_StartingLoc;
cLocation			cMasterServer::m_MarketLoc;
CorpseCleaner*		cMasterServer::m_pcCorpse			= 0;
SimpleAI*			cMasterServer::m_pcSimpleAI			= 0;
FILE*				cMasterServer::pFile;
CStatus*			cMasterServer::cStatus				= 0;
char				cMasterServer::m_szServerName[64]	= { ' ', };
DWORD				cMasterServer::m_dwNumUsers			= 0;
DWORD				cMasterServer::m_UserCount			= 0;
std::list< cEnchantment * > cMasterServer::m_lstEnchantments;
std::vector< cTeleTownList >  cMasterServer::m_TeleTownList;
std::vector< cAllegiance *>  cMasterServer::m_AllegianceList;
std::vector< cFellowship *>  cMasterServer::m_FellowshipList;
DWORD				cMasterServer::m_dwFellowID			= 0;
TreasureGen *trgen = new TreasureGen();
DWORD				gNPCCount							= 0;

cNPC* npc;

friend class cClient;

void cMasterServer::WriteToFile( char *szMessage )
{
	if(pFile == NULL)
	{
	}
	else
	{
		fprintf(pFile,szMessage );
		fprintf(pFile,"\r\n");
	}
}

/**
 *	Loads the server process.  Calls cMasterServer::StartThread( ).
 *
 *	This member function is called when the server is started.
 */
void cMasterServer::Load( )
{
	if( m_iLoadCount > 0 )
	{
		UpdateConsole( "Server already running!\r\n" );
		return;
	}
	else
	{
		UpdateConsole( " --------------------------------------------------------------------------\r\n\r\n" );
		UpdateConsole( " Starting Master Server ...\r\n" );
		
	}

	m_iLoadCount++;

	pFile = fopen( "server.log", "wa" );
	setbuf(pFile,NULL);
	
	//Karki
	cMasterServer::m_UserCount = 0;

	if( pFile == NULL )
	{
		UpdateConsole( " Cannot open c:\\server.log \r\n" );
		//exit( 1 );
	}
	else
	{
		WriteToFile("Master Server started.\r\n");
		UpdateConsole( " Master Server started.\r\n\r\n" );
	}


	StartThread( );
}
/**
 *	Unloads the server process.  Calls cMasterServer::StopThread( ).
 *
 *	This member function is called when the server is stopped.
 */
BOOL cMasterServer::Unload( )
{
	m_iLoadCount--;
	WriteToFile("Program exiting ...");
	if(pFile == NULL)
	{
	}
	else
	{
		fclose(pFile);
	}

	if( m_iLoadCount == 0 )
	{

		UpdateConsole( "\r\n Stopping Master Server. Please wait ...\r\n" );

		StopThread( 5000 );
		
		m_iLoadCount = 0;

		UpdateConsole( " All services stopped.\r\n\r\n" );
		/////////////////////////////
		cStatus->ServerShutdown();
		/////////////////////////////
		cWorldManager::RemoveAllObjects();
		cWorldManager::Unload();
		return TRUE;
	}
	else {
		WriteToFile("Master Server services failed to stop.");
		UpdateConsole( " Master Server services failed to stop.\r\n\r\n" );
		return FALSE;
	}
}

/**
 *	Clears all spawned objects.
 */
void cMasterServer::ClearAllObjects( )
{
	PostThreadMessage( m_dwThreadID, WM_CLEAROBJECTS, 0, 0 );
}

/**
 *	Starts the MasterServer thread.
 *
 *	This member function is called when the MasterServer thread is to be started.
 */
void cMasterServer::StartThread( )
{
	m_hExitEvent	= CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hThread		= CreateThread( NULL, NULL, ServerThread, (LPVOID)NULL, NULL, &m_dwThreadID );
}

/**
 *	Cleans up and closes the MasterServer thread.
 *
 *	This member function is called when the MasterServer thread is to be stopped.
 */
void cMasterServer::StopThread( DWORD dwTimeOut )
{
	SetEvent( m_hExitEvent );
	MSG msg;

	BOOL fContinue = TRUE;

	DWORD dwKillTime = timeGetTime( );

	while ( fContinue )
	{
		switch( MsgWaitForMultipleObjects( 1, &m_hThread, FALSE, dwTimeOut, QS_ALLINPUT ) )
		{
			case WAIT_OBJECT_0:
			{
				fContinue = FALSE;
				break;
			}

			case WAIT_OBJECT_0 + 1:
			{
				while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
					{ 
						if( IsDialogMessage( g_hWndMain, &msg ) )
							continue;

						TranslateMessage( &msg );
						DispatchMessage( &msg ); 
					}
			
				if( ( timeGetTime( ) - dwKillTime ) > dwTimeOut )
					dwTimeOut = 0;
				else
					break;
			}
			
			case WAIT_TIMEOUT:
			{
				UpdateConsole( " Thread time-out. Killing the thread...\r\n" );
				TerminateThread( m_hThread, 0 );
				fContinue = FALSE;
				break;
			}
		}
	}

	CloseHandle( m_hThread );
	CloseHandle( m_hExitEvent );
}

/**
 *	The thread loop that checks for new packets and passes them on to the correct server.
 */
DWORD WINAPI cMasterServer::ServerThread( LPVOID lpVoid )
{

	cDatabase::Load( );
	cDatabase::InitializeMaxModel( );

	LoadStartingLocation( );
	LoadTeleTownList( );
	LoadAllegiances( );

	cCharacterServer::Initialize( g_nCharPort, 0x0B, 0x56FC18E1, 0x53575F2D );
	cWorldServer::Initialize( g_nWorldPort, 0x51, 0xD6748919, 0xC01CA84F );
	cMonsterServer::Initialize( );
	cClient::Hash_Load( );

	m_pcSimpleAI	= new SimpleAI( );

	cWorldManager::Load( );


	m_pcJobPool		= new cJobPool( );
	m_pcJobPool->NextTickTime = clock();

	m_pcCorpse		= new CorpseCleaner( );
	

	m_uipTimer = SetTimer( NULL, NULL, 10000, cMasterServer::ScavengeIdleSockets );
	m_uipStatusTimer = SetTimer( NULL, NULL, 20000, cMasterServer::Status_Update );

	HANDLE				hEvents[3];
	WSANETWORKEVENTS	NetEvents;
	cRecvPacket			cRP;

	BOOL fActive			= TRUE;
	
	int		iSockSize		= sizeof( sockaddr_in );
	DWORD	dwWaitTimeOut	= 50;
	int		iRPDataSize		= sizeof( cRP.m_bData );
	MSG		msg;
	int		PetCheckNoobCount = 0;

	///////
	cStatus->ServerStart();
	///////

	hEvents[0]	= m_hExitEvent;
	hEvents[1]	= WSACreateEvent( );
	hEvents[2]	= WSACreateEvent( );

	WSAEventSelect( cWorldServer::m_Socket, hEvents[1], FD_READ );
	WSAEventSelect( cCharacterServer::m_Socket, hEvents[2], FD_READ );

	while ( fActive )
	{

		time(&m_pcCorpse->longtime);
		if(m_pcCorpse->NextCleanTime < m_pcCorpse->longtime)
		{
			m_pcCorpse->Cleanup();
		}
		time(&m_pcSimpleAI->longtime);

		if(m_pcSimpleAI->NextActionTime < m_pcSimpleAI->longtime)
		{
			m_pcSimpleAI->ExecuteActions( );
		}

		switch ( MsgWaitForMultipleObjects( 3, hEvents, FALSE, dwWaitTimeOut, QS_TIMER | QS_POSTMESSAGE ) )
		{
			case ( WAIT_OBJECT_0 ):
			{
				fActive = FALSE;
				break;	// case WAIT_OBJECT_0
			}

			case ( WAIT_TIMEOUT ):
			{			
				m_pcJobPool->longtime = clock();
				if(m_pcJobPool->longtime - m_pcJobPool->NextTickTime >= (CLOCKS_PER_SEC/TICKS_PER_SEC) )
				{
					m_pcJobPool->NextTickTime += (CLOCKS_PER_SEC/TICKS_PER_SEC);
					//cMasterServer::ServerMessage(ColorGreen,NULL,"Tick: %d, %d", m_pcJobPool->NextTickTime, m_pcJobPool->longtime);
					m_pcJobPool->Tick( );
					m_pcSimpleAI->MoveMonsters();
				}
				for ( iterEnchantment_lst itEnchantment = m_lstEnchantments.begin() ; itEnchantment != m_lstEnchantments.end() ; ++itEnchantment )
				{
					if ( (*itEnchantment)->m_dCastTime + (*itEnchantment)->m_dDuration * 1000 < timeGetTime() )
					{
						SAFEDELETE ( *itEnchantment )
						itEnchantment = m_lstEnchantments.erase( itEnchantment );
					}
				}
				dwWaitTimeOut = 50;
				cClient::SendOffAllPackets( );
				break;	// case WAIT_TIMEOUT
			}
			
			case ( WAIT_OBJECT_0 + 1 ):
			{
				WSAEnumNetworkEvents( cWorldServer::m_Socket, hEvents[1], &NetEvents );

				switch ( NetEvents.lNetworkEvents )
				{						
					case FD_READ:
					{
						dwWaitTimeOut = 0;

						cRP.m_wSize = recvfrom( cWorldServer::m_Socket, (char *)cRP.m_bData, iRPDataSize, NULL, (sockaddr *)&cRP.m_saSockAddr, &iSockSize );
								
						if ( (cRP.m_wSize > 0) && (cRP.m_wSize != SOCKET_ERROR) )
						{
							//cClient::Hash_New( cRP.m_saSockAddr )->ProcessPacket_WS( &cRP );
							cClient *pcClient = cClient::Hash_Find( cRP.m_saSockAddr );
									
							// If a client the client does not exist, or has an invalid
							// avatar, do not continue.
							if ( !pcClient || !pcClient->m_pcAvatar )
								break;	// case FD_READ

							pcClient->ProcessPacket_WS( &cRP );
						}
						break;	// case FD_READ
					}
				}
			
				break;	//case WAIT_OBJECT_0 + 1
			}

			case ( WAIT_OBJECT_0 + 2 ):
			{
				WSAEnumNetworkEvents( cCharacterServer::m_Socket, hEvents[2], &NetEvents );

				switch ( NetEvents.lNetworkEvents )
				{						
					case FD_READ:
					{
						dwWaitTimeOut = 0;

						cRP.m_wSize = recvfrom( cCharacterServer::m_Socket,(char *)cRP.m_bData, iRPDataSize, NULL,(sockaddr *)&cRP.m_saSockAddr, &iSockSize );
								
						if ( (cRP.m_wSize > 0) && (cRP.m_wSize != SOCKET_ERROR) )
							cClient::Hash_New( cRP.m_saSockAddr )->ProcessPacket_CS( &cRP );

						break;	// case FD_READ
					}
				}
				
				break;	// case WAIT_OBJECT_0 + 2
			}
			
			case (WAIT_OBJECT_0 + 3 ):
			{
				while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{
					if( msg.message == WM_CLEAROBJECTS )
					{
						cWorldManager::RemoveAllObjects();
						cMasterServer::ServerMessage(ColorGreen,NULL,"The server administrator has cleared all the objects.");
					}

					DispatchMessage( &msg );
				}
				
				break;	// case WAIT_OBJECT_0 + 3
			}
		}
	}


	CloseHandle( hEvents[1] );
	CloseHandle( hEvents[2] );

	KillTimer( NULL, m_uipTimer );

	DisconnectAllClients( );
		
	SAFEDELETE( m_pcJobPool )

	cCharacterServer::Halt( );
	cWorldServer::Halt( );
	cWorldManager::Unload( );
	cDatabase::Unload( );

	/////////////////
	cStatus->ServerShutdown();
	////////////////
	return 0;
}

/**
 *	Sends a server message to the client.
 *
 *	@param dwColor - The color in which the server message will be displayed.
 *	@param *pcClient - A pointer to the client who should receieve the message.
 *	@param *szMessage - A pointer to the text to be sent as the message.
 */
void cMasterServer::ServerMessage( DWORD dwColor, cClient *pcClient, char *szMessage, ... )
{
	char	szTextBuffer[1024];

	va_list	valMaker;
	
	va_start( valMaker, szMessage );
	wvsprintf( szTextBuffer, szMessage, valMaker );

	cMessage cmSM;
	cmSM << 0xF62CL << szTextBuffer << dwColor;

	if ( pcClient == NULL )
		cClient::SendToAllClients( cmSM, 4 );
	else
		pcClient->AddPacket( WORLD_SERVER, cmSM, 4 );
}

/**
 *	Obtains the avatar's information from the database and creates the client's avatar.
 *
 *	This function is called whenever a client logs in an avatar.
 *	
 *	Note:  Placeholder code until database code is done.
 *
 *	@param **ppcAvatar - A pointer to the pointer to avatar to be created.
 *	@param dwGUID - The GUID of the avatar.
 */
void cMasterServer::CreateNewAvatar( cAvatar **ppcAvatar, DWORD dwGUID )
{
	*ppcAvatar = new cAvatar( dwGUID, &m_StartingLoc );
	cDatabase::LoadAvatar( *ppcAvatar );
}

/**
 *	Obtains the data necessary from the database for login and sends it to the client.
 *
 *	This function is called whenever a client logs in an avatar.
 *	It is called after the database contents for the avatar have been loaded into memory.
 *	
 *	Note:  Placeholder code until database code is done.
 *
 *	@param *pcClient - A pointer to the client that should receive the data.
 */
void cMasterServer::SendLoginData( cClient *pcClient )
{
	ServerMessage( ColorCyan, pcClient, "To view a list of commands, type !help" );
	ServerMessage( ColorLightPink, pcClient, "Note: If you are stuck in portal mode, type \"/render radius 5\"" );
	
	// Character data
	DWORD dwGUID = pcClient->m_pcAvatar->GetGUID( );
	
	// 0x13 Login
	pcClient->AddPacket( WORLD_SERVER, pcClient->m_pcAvatar->CreateLoginPacket(++pcClient->m_dwF7B0Sequence), 4);

	// Pack contents
	pcClient->AddPacket( WORLD_SERVER, pcClient->m_pcAvatar->SetPackContents(++pcClient->m_dwF7B0Sequence), 4 );

	// Housing Info
	pcClient->AddPacket( WORLD_SERVER, pcClient->m_pcAvatar->HousingInfo(++pcClient->m_dwF7B0Sequence), 4);

	// Creation packet/CreateObject
	pcClient->AddPacket( WORLD_SERVER, pcClient->m_pcAvatar->CreatePacket( ), 3 );

	// Exit Portal Mode
	pcClient->AddPacket( WORLD_SERVER, pcClient->m_pcAvatar->LoginCharacter( ), 3 );

	//Karki
	ServerMessage( ColorMagenta, NULL, "%s has joined %s. %d client(s) connected.", pcClient->m_pcAvatar->Name( ), cMasterServer::m_szServerName, ++cMasterServer::m_UserCount);
	//EndKarki

	pcClient->m_pcAvatar->SetArmorLevel(0);
}

/**
 *	Parses commands from users and executes them.
 *
 *	Used for server-specific commands.
 *	Executes the proper cCommandParser function based upon the command.
 *
 *	@param *szCommand - A pointer to the text that comprises the command.
 *	@param wSize - The length of the command (excluding the terminating character ('\0')
 *	@param *pcClient - A pointer to the client that should receive the data.
 */
void cMasterServer::ParseCommand( char *szCommand, WORD wSize, cClient *pcClient )
{
	cCommandParser::m_pcClient = pcClient;

	if ( *szCommand == '!' )
	{
		char *szCommandName = strtok( szCommand, " ," );
		++szCommandName;
		
		WORD wCommandSize = lstrlen( szCommandName ) + 2;
		if( wCommandSize < wSize )
			szCommand += wCommandSize;
		else
			*szCommand = 0;

		switch ( pcClient->m_pcAvatar->m_bAccessLevel )
		{
		
		case eDeveloper:
			if ( !lstrcmpi( szCommandName, "teleloc" ) )				cCommandParser::TeleLoc( szCommand );
			if ( !lstrcmpi( szCommandName, "telemap" ) )				cCommandParser::Telemap( szCommand );
			if ( !lstrcmpi( szCommandName, "teletown" ) )				cCommandParser::TeleTown( szCommand );
			if ( !lstrcmpi( szCommandName, "turn" ) )					cCommandParser::Turn( szCommand );
		case eAdmin:
			if ( lstrcmpi( szCommandName, "spawnsave" ) == 0 )			cCommandParser::SpawnSave( szCommand );
			else if ( lstrcmpi( szCommandName, "spawnmonster" ) == 0 )	cCommandParser::SpawnMonster( szCommand );
			else if ( lstrcmpi( szCommandName, "spawntype" ) == 0 )		cCommandParser::Spawntype( szCommand );
			else if ( !lstrcmpi( szCommandName, "SpawnItem") )			cCommandParser::SpawnItem( szCommand );
			else if ( !lstrcmpi( szCommandName, "SpawnItemLB") )		cCommandParser::SpawnItemLB (szCommand);
			else if ( !lstrcmpi( szCommandName, "RandomPyreals") )		cCommandParser::RandomPyreals();
			else if ( !lstrcmpi( szCommandName, "particle" ) )			cCommandParser::Particle( szCommand );
			else if ( !lstrcmpi( szCommandName, "sound" ) )				cCommandParser::SoundEffect( szCommand );
		case eUeber:
			if ( !lstrcmpi( szCommandName, "clearobjects" ) )			cCommandParser::ClearObjects( );
			else if ( !lstrcmpi( szCommandName, "wb" ) )				cCommandParser::WorldBroadcast( szCommand );
		case eSentinel:
		case eAdvocate:
			if ( !lstrcmpi( szCommandName, "getchar" ) )			cCommandParser::GetCharacter( szCommand );
			else if ( !lstrcmpi( szCommandName, "gotochar" ) )			cCommandParser::GotoCharacter( szCommand );
			else if ( !lstrcmpi( szCommandName, "returnchar" ) )		cCommandParser::ReturnCharacter( szCommand );
			else if ( !lstrcmpi( szCommandName, "sendchar" ) )			cCommandParser::SendCharacter( szCommand );
			else if ( !lstrcmpi( szCommandName, "spawnid") )			cCommandParser::SpawnID( szCommand );
		case eStaff:
		case eVIP:
		case eNormal:
		default:
			if ( !lstrcmpi( szCommandName, "help" ) )					cCommandParser::Help( szCommand, pcClient->m_pcAvatar->m_bAccessLevel );	
			else if ( !lstrcmpi( szCommandName, "global" ) )			cCommandParser::GlobalChat( szCommand );
			else if ( !lstrcmpi( szCommandName, "g" ) )					cCommandParser::GlobalChat( szCommand );
			else if ( !lstrcmpi( szCommandName, "shout" ) )				cCommandParser::GlobalChat( szCommand );
			else if ( !lstrcmpi( szCommandName, "s" ) )					cCommandParser::GlobalChat( szCommand );
			else if ( !lstrcmpi( szCommandName, "home" ) )				cCommandParser::Home();
			else if ( !lstrcmpi( szCommandName, "invisible") )			cCommandParser::Invisible( );
			else if ( !lstrcmpi( szCommandName, "visible") )			cCommandParser::Visible( );
			else if ( !lstrcmpi( szCommandName, "who") )				cCommandParser::Who( );
			else if ( !lstrcmpi( szCommandName, "animation" ) )			cCommandParser::Animation( szCommand );
			else if ( !lstrcmpi( szCommandName, "setmodel" ) )			cCommandParser::SetModel( szCommand );
			else if ( !lstrcmpi( szCommandName, "setscale" ) )			cCommandParser::SetScale( szCommand );
			else if ( !lstrcmpi( szCommandName, "version") )			cCommandParser::Version( );

			else if ( !lstrcmpi( szCommandName, "boot") )
			
			{
				cMessage cMsg;
				cMsg << 0xF7B0L << pcClient->m_pcAvatar->GetGUID() << ++pcClient->m_dwF7B0Sequence << 0x01C8L << 0x00L << 0x35L;
				cMsg << 0x0229L << BYTE(0x00) << pcClient->m_pcAvatar->GetGUID() << 0x0L << 0xFFFFFFFF;
				pcClient->AddPacket(WORLD_SERVER,cMsg,4);
			}
			else if ( !lstrcmpi( szCommandName, "...") )			cCommandParser::ODOA( );
			//Karki
		}
		//else if ( lstrcmpi( szCommandName, "munster" ) == 0 )		Munster( );
		//else if ( lstrcmpi( szCommandName, "dungeon" ) == 0 )		Dungeon( szCommand );
		//else if ( lstrcmpi( szCommandName, "punch" ) == 0 )		Punch( );
		//else if ( lstrcmpi( szCommandName, "dungeonlist" ) == 0 )	DungeonList( szCommand );
		//else if ( lstrcmpi( szCommandName, "spawn" ) == 0 )		SpawnMonster( szCommand );
		//else if ( lstrcmpi( szCommandName, "spawnplayer" ) == 0 )	SpawnPlayer( );
		//else if ( lstrcmpi( szCommandName, "portal" ) == 0 )		SpawnPortal( );
		//else if ( lstrcmpi( szCommandName, "spawnid") == 0 )		SpawnID(szCommand);
		//else if ( lstrcmpi( szCommandName, "npcsave" ) == 0 )		cCommandParser::SaveNPC( szCommand );
		//else if ( !lstrcmpi( szCommandName, "Wear") )				cCommandParser::Wear( );
		//else if ( !lstrcmpi( szCommandName, "Remove") )				cCommandParser::Remove( );
		//else if ( !lstrcmpi( szCommandName, "acid") )				cCommandParser::Acid( szCommand );
		//else if ( !lstrcmpi( szCommandName, "recordloc" ) )			cCommandParser::RecordLocation( ); // szCommand );
		//else if ( !lstrcmpi( szCommandName, "spwnid") )				cCommandParser::SpwnID( szCommand );
	}
	else
	{
		if ( *szCommand == '@' )
		{
		}
		else
		{
		cMessage cmChat;

		cmChat	<< 0x0037L << szCommand << (char *)pcClient->m_pcAvatar->Name( )
				<< pcClient->m_pcAvatar->GetGUID( ) << 0x02;
		
		cWorldManager::SendToAllWithin( 1, pcClient->m_pcAvatar->m_Location, cmChat, 4 );
		}
	}
}

/**
 *	Encapsulates functionality to send a private message to another user
 *
 *	This function is called whenever a client sends a "tell" to another client.
 *
 *	@param *szCommand - A pointer to the text that comprises the command.
 *	@param *pcDestination - A pointer to the client that should receive the tell.
 *	@param *pcOrigin - A pointer to the client sending the the tell.
 */
void cMasterServer::SendTell( char *szMessage, cClient *pcDestination, cClient *pcOrigin )
{
	if ( !pcDestination )
	{
		cMessage cmNA;
		cmNA	<< 0xF7B0L << pcOrigin->m_pcAvatar->GetGUID( ) << ++pcOrigin->m_dwF7B0Sequence
				<< 0x0016L << "Talking to your invisible friend again?";

		pcOrigin->AddPacket( WORLD_SERVER, cmNA, 4 );
		return ;
	}

	cMessage cmChat;
	DWORD dwDestinationGUID = pcDestination->m_pcAvatar->GetGUID( );


	cmChat	<< 0xF7B0L << dwDestinationGUID << ++pcDestination->m_dwF7B0Sequence 
			<< 0x0038L << szMessage << pcOrigin->m_pcAvatar->Name( ) 
			<< pcOrigin->m_pcAvatar->GetGUID( ) << dwDestinationGUID << 0x04L;

	pcDestination->AddPacket( WORLD_SERVER, cmChat, 4 );

	if ( pcDestination != pcOrigin )
		ServerMessage( ColorBrown, pcOrigin, "You tell %s, \"%s\"", pcDestination->m_pcAvatar->Name( ), szMessage );
}

/**
 *	Removes a client after logout or loss of signal.
 *
 *	@param *pcClient - A pointer to the client to be disconnected.
 */
BOOL cMasterServer::DisconnectClient( cClient *pcClient )
{

	char szIP[32];
	if (pcClient->m_wAccountNameLength < 40) {
		FormatIP_Port( pcClient->m_saSockAddr, szIP );
		UpdateConsole(	" Client logout:\r\n"
						"      User: %s\r\n"
						"      IP: %s\r\n", pcClient->m_szAccountName, szIP );
	}
		cClient::Hash_Remove( pcClient );
	
	return TRUE;
}

/**
 *	Changes a Client's PKLite status.
 *
 *	@param *pcClient - A pointer to the client whose PKLite status is to be changed.
 *	@param bState - A boolean representation of whether the client is PKLite.
 */
BOOL cMasterServer::PKLite( cClient *pcClient, bool bState )
{
	if (bState == true)
	{
		char szNotice[200];
		wsprintf( szNotice, "%s is looking for a fight!",pcClient->m_pcAvatar->Name( ) );
		
		cMessage cmNotice;
		cmNotice << 0xF62C << szNotice << ColorGreen;
		cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, cmNotice, 4 );

		cMasterServer::ServerMessage( ColorBlue, pcClient, "A cold wind touches your heart. You are now a Player Killer Lite.");
		cMessage cmCoverME;
		cmCoverME << 0x0229L << pcClient->m_pcAvatar->m_bFlagCount++<< pcClient->m_pcAvatar->GetGUID() << DWORD(134) << 0x40L;
		cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, cmCoverME, 4 );
		pcClient->m_pcAvatar->m_fIsPK = 2;
	}
	else
	{
		cMessage cmCoverME;
		cmCoverME << 0x0229L << pcClient->m_pcAvatar->m_bFlagCount++ << pcClient->m_pcAvatar->GetGUID() << DWORD(134) << 0x0L;
		cWorldManager::SendToAllWithin( 5 , pcClient->m_pcAvatar->m_Location, cmCoverME, 4 );
		pcClient->AddPacket(WORLD_SERVER,cmCoverME,4);
		pcClient->m_pcAvatar->m_fIsPK = 0;
	}

	return bState;
}

/**
 *	Removes all clients in preparation for server shutdown.
 */
BOOL cMasterServer::DisconnectAllClients( )
{
	UpdateConsole( " Disconnecting all clients ...\r\n" );
	ServerMessage( ColorBlue, NULL, "The server has been shut down." );
	cClient::SendOffAllPackets( );

	cClient::Hash_Erase( );

	return TRUE;
}

/**
 *	Searches for and disconnects inactive clients.
 */
VOID CALLBACK cMasterServer::ScavengeIdleSockets( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{	
	cClient *pcClient, *pcPrevClient;
	DWORD dwCurrentTime = timeGetTime( );

	for ( int i = 0; i < 1020; ++i )
	{
		pcClient = cClient::m_lpcHashTable[i];

		while ( pcClient ) 
		{	
			pcPrevClient	= pcClient;
			pcClient		= pcClient->m_pcNext;

			if ( dwCurrentTime - pcPrevClient->m_dwLastRecvTime > 40000 )
				DisconnectClient( pcPrevClient );
		}
	}
}

/**
 *	Updates the Master Server.
 *
 *	Author: G70mb2
 */
VOID CALLBACK cMasterServer::Status_Update( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{	
	DWORD dwCurrentTime = timeGetTime( );	
	//UpdateConsole( " Update status timer check.\r\n" );
	if( dwCurrentTime - cMasterServer::cStatus->m_ulLastUpdate > 200000 )
	{
		cMasterServer::cStatus->m_cAvg = m_dwNumUsers;
		//UpdateConsole( " Update time has been reached.\r\n");
		cMasterServer::cStatus->UpDate();
	}
}

/**
 *	Converts '_' into a space character.
 */
void cMasterServer::FixSpaces(char* str)
{
	for(int a = 0;a<strlen(str);a++)
	{
		if(str[a] == '_')
		{
			str[a] = 0x20;
		}
	}
}

/**
 *	Converts * into a ' (apostrophe).
 */
void cMasterServer::FixName(char* str)
{
	for(int a = 0;a<strlen(str);a++)
	{
		if(str[a] == '*')
		{
			str[a] = 0x27;
		}
	}
}

BOOL cMasterServer::FindHeaderInFile( FILE* fin, char* header )
{
	char findstr[100];
	wsprintf(findstr,"[%s]\n",header);
	rewind(fin);
	while(!feof(fin))
	{
		char inbuf[300];
		fgets(inbuf,299,fin);
		if(strcmp(inbuf,findstr)==0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 *	Initalizes an avatar's inventory by loading each item from the database.
 *
 *	This function should be called before the avatar's Create Message is formed.
 *	The information loaded here is used in the Create Message.
 *
 *	@param *pcClient - A pointer to the client whose avatar's inventory is to be loaded.
 */
void cMasterServer::CreateInventory( cClient *pcClient )
{
   //The avatar's GUID
   DWORD avatarGUID = pcClient->m_pcAvatar->GetGUID( );

   char szCommand [200];
   RETCODE retcode;

   std::list< DWORD >   lstInventory;

   DWORD dwObjectGUID;
   DWORD   fEquipped;
   char   data[512];

   DWORD   dwItemModelNumber;
   DWORD   dwItemAmount;
   WORD   wIcon;
   int      intColor;
   char   spells[255];

   sprintf( szCommand, "SELECT GUID FROM items_instance_inventory WHERE OwnerGUID=%lu;      ", avatarGUID );

   retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );                           CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
   retcode = SQLExecute( cDatabase::m_hStmt );                                                CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
   retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwObjectGUID, sizeof( DWORD ), NULL );      CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
   for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
   {
      lstInventory.push_back( dwObjectGUID );
   }

   for ( std::list<DWORD>::iterator list_iter = lstInventory.begin(); list_iter != lstInventory.end(); ++list_iter )
   {
      sprintf( szCommand, "SELECT GUID,Equipped,data FROM items_instance_inventory WHERE GUID=%lu;      ", *list_iter );

      retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );                        CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
      retcode = SQLExecute( cDatabase::m_hStmt );                                             CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
      retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwObjectGUID, sizeof( DWORD ), NULL );   CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
      retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_LONG, &fEquipped, sizeof( DWORD ), NULL );      CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
      retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_CHAR, &data, sizeof( data ), NULL );            CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
      
      if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS )
      {
         sscanf(data,"%d %d %d %d %s",&dwItemModelNumber,&dwItemAmount,&intColor,&wIcon,&spells);

         retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
         retcode = SQLCloseCursor( cDatabase::m_hStmt );            CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
         retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

         //Find the model
         cItemModels *pcModel = cItemModels::FindModel(dwItemModelNumber);

         switch(pcModel->m_ItemType)
         {
            case 1:
            {
//            cWeapon* aWeapon = new cWeapon(pcClient->m_pcAvatar->m_vInventory[i].dwObjectGUID,avatarGUID,dwItemModelNumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wBurden,pcModel->m_dwValue,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->);
//            cWorldManager::AddObject(aWeapon,true);
//            pcClient->AddPacket( WORLD_SERVER, aWeapon->CreatePacketContainer(avatarGUID,pcModel->m_dwModelID),3);
//            pcClient->m_pcAvatar->AddInventory(aWeapon);
            cWeapon* aWeapon = new cWeapon(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->m_dwWeaponDamage,pcModel->m_dwWeaponSpeed,pcModel->m_dwWeaponSkill,pcModel->m_dwDamageType,pcModel->m_dWeaponVariance,pcModel->m_dWeaponModifier,pcModel->m_dWeaponPower,pcModel->m_dWeaponAttack);
            aWeapon->m_fEquipped = fEquipped;
            LoadItem(aWeapon);
            pcClient->m_pcAvatar->AddInventory(aWeapon);
            pcClient->AddPacket( WORLD_SERVER, aWeapon->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
            break;
            }
            
            case 2:
            {
            cFood* aFood = new cFood(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wStack, pcModel->m_wBurden, pcModel->m_dwVitalID, pcModel->m_vital_affect);
            aFood->m_fEquipped = fEquipped;
            LoadItem(aFood);
            pcClient->m_pcAvatar->AddInventory(aFood);
            pcClient->AddPacket( WORLD_SERVER, aFood->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
            break;
            }

            case 3:
            {
            //cArmor* aArmor = new cArmor(cWorldManager::NewGUID_Object(),pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_dwArmor_Level,pcModel->m_fProt_Slashing,pcModel->m_fProt_Piercing,pcModel->m_fProt_Bludgeon,pcModel->m_fProt_Fire,pcModel->m_fProt_Cold,pcModel->m_fProt_Acid,pcModel->m_fProt_Electric);
            cArmor* aArmor = new cArmor(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_dwArmor_Level,pcModel->m_fProt_Slashing,pcModel->m_fProt_Piercing,pcModel->m_fProt_Bludgeon,pcModel->m_fProt_Fire,pcModel->m_fProt_Cold,pcModel->m_fProt_Acid,pcModel->m_fProt_Electric);
            aArmor->m_fEquipped = fEquipped;
            if (aArmor->m_fEquipped == 2)
            {
               aArmor->m_dwOwnerID = pcClient->m_pcAvatar->GetGUID();
            }
            aArmor->m_intColor = intColor;
            LoadItem(aArmor);
            pcClient->m_pcAvatar->AddInventory(aArmor);
            pcClient->AddPacket( WORLD_SERVER, aArmor->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
            break;
            }

			case 4:
				{
					cBooks* Book = new cBooks(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
					Book->m_fEquipped = fEquipped;
					LoadItem(Book);
					pcClient->m_pcAvatar->AddInventory(Book);
					pcClient->AddPacket(WORLD_SERVER,Book->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
					break;
				}

			case 6:
				{
					cHealingCon *kit = new cHealingCon(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wBurden,pcModel->m_dwValue,pcModel->m_wUses,pcModel->m_wUseLimit);
					kit->m_fEquipped = fEquipped;
					LoadItem(kit);
					pcClient->m_pcAvatar->AddInventory(kit);
					pcClient->AddPacket(WORLD_SERVER,kit->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
					break;
				}
			
			case 7:
				{
					cLockpicks* Picks = new cLockpicks(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wUses, pcModel->m_wUseLimit);
					Picks->m_fEquipped = fEquipped;
					LoadItem(Picks);
					pcClient->m_pcAvatar->AddInventory(Picks);
					pcClient->AddPacket(WORLD_SERVER,Picks->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
					break;
				}

            case 8:
            {
            cWands* aWand = new cWands(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
            aWand->m_fEquipped = fEquipped;
            LoadItem(aWand);
            pcClient->m_pcAvatar->AddInventory(aWand);
            pcClient->AddPacket( WORLD_SERVER, aWand->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
            break;
            }
            /*
			case 9:
				{
					cPyreals* Cash = new cPyreals(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,dwItemAmount,25000);
					Cash->m_fEquipped = fEquipped;
					LoadItem(Cash);
					pcClient->m_pcAvatar->AddInventory(Cash);
					pcClient->AddPacket( WORLD_SERVER, Cash->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
					break;
				}
			*/

			case 11:
				{
					cAmmo* Ammo = new cAmmo(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,dwItemAmount,dwItemAmount);
					Ammo->m_fEquipped = fEquipped;
					LoadItem(Ammo);
					pcClient->m_pcAvatar->AddInventory(Ammo);
					pcClient->AddPacket( WORLD_SERVER, Ammo->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
					break;
				}
            case 13:
            {
            cSpellComps* aComps = new cSpellComps(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, dwItemAmount ,pcModel->m_wBurden);
            aComps->m_fEquipped = fEquipped;
            LoadItem(aComps);
            pcClient->m_pcAvatar->AddInventory(aComps);
            pcClient->AddPacket( WORLD_SERVER, aComps->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
            break;
            }

			case 14:
				{
					cGems* aGem = new cGems(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
					aGem->m_fEquipped = fEquipped;
					LoadItem(aGem);
					pcClient->m_pcAvatar->AddInventory(aGem);
					pcClient->AddPacket( WORLD_SERVER, aGem->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
					break;
				}

            case 18:
            {
            //cClothes* aClothes = new cClothes(cWorldManager::NewGUID_Object(),pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
            cClothes* aClothes = new cClothes(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
            aClothes->m_fEquipped = fEquipped;
            if (aClothes->m_fEquipped == 2)
            {
               aClothes->m_dwOwnerID = pcClient->m_pcAvatar->GetGUID();
            }
            aClothes->m_intColor = intColor;
            LoadItem(aClothes);
            pcClient->m_pcAvatar->AddInventory(aClothes);
            pcClient->AddPacket( WORLD_SERVER, aClothes->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
            break;
            }

            case 22:
            {
            //cFoci* aFoci = new cFoci(cWorldManager::NewGUID_Object(),pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
            cFoci* aFoci = new cFoci(dwObjectGUID,pcClient->m_pcAvatar->GetGUID(),dwItemModelNumber,1.0,TRUE,wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
            aFoci->m_fEquipped = fEquipped;
            LoadItem(aFoci);
            pcClient->m_pcAvatar->AddInventory(aFoci);
			pcClient->AddPacket( WORLD_SERVER, aFoci->CreatePacketContainer(pcClient->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
            break;
            }
         }
      } else {
         retcode = SQLCloseCursor( cDatabase::m_hStmt );            CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
         retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
      }
   }

   lstInventory.clear();
}



/**
 *	Creates a corpse upon user's death.
 *
 *	@param *pcClient - A pointer to the client creating the corpse.
 *
 *	Author: G70mb2
 */
BOOL cMasterServer::Corpse( cClient *pcClient )
{

	DWORD dwModel = 2185;
	DWORD dwIcon = 1024;
	float flScale = 1.0;
	BOOL fSolid = 0;
	BOOL fSelectable = 1;
	BOOL fEquippable = 0;
	char szName[60] = { ' ',};

	std::ostringstream ssDescription;
	ssDescription << "Corpse";
	sprintf( szName, "Corpse");

	cAbiotic *pcModel = new cAbiotic( cWorldManager::NewGUID_Object( ), pcClient->m_pcAvatar->m_Location, dwModel, flScale, fSolid, dwIcon, szName, ssDescription.str( ), 500, 2500, fSelectable, fEquippable );
	pcModel->SetStatic( FALSE );
	CorpseCleaner::AddCorpse( pcModel->GetGUID(),120,0);
	cWorldManager::AddObject( pcModel );

	return TRUE;
}

/**
 *	Obtains the default starting location from the database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadStartingLocation( )
{
	char		szCommand[512];
	RETCODE		retcode;
	char		readbuff[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];

	sprintf( szCommand, "SELECT * FROM starting_locations WHERE LocID=1;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 3;

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, readbuff, sizeof( readbuff ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_StartingLoc.m_flX, sizeof( m_StartingLoc.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_StartingLoc.m_flY, sizeof( m_StartingLoc.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_StartingLoc.m_flZ, sizeof( m_StartingLoc.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_StartingLoc.m_flA, sizeof( m_StartingLoc.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_StartingLoc.m_flB, sizeof( m_StartingLoc.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_StartingLoc.m_flC, sizeof( m_StartingLoc.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_StartingLoc.m_flW, sizeof( m_StartingLoc.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	retcode = SQLFetch( cDatabase::m_hStmt );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	sscanf(readbuff,"%08x",&m_StartingLoc.m_dwLandBlock);
	sscanf(dwPosX,"%08x",&m_StartingLoc.m_flX);
	sscanf(dwPosY,"%08x",&m_StartingLoc.m_flY);
	sscanf(dwPosZ,"%08x",&m_StartingLoc.m_flZ);
	sscanf(dwOrientW,"%08x",&m_StartingLoc.m_flA);
	sscanf(dwOrientX,"%08x",&m_StartingLoc.m_flB);
	sscanf(dwOrientY,"%08x",&m_StartingLoc.m_flC);
	sscanf(dwOrientZ,"%08x",&m_StartingLoc.m_flW);

	UpdateConsole( " Starting locations loaded.\r\n" );

	// Load Marketplace Location
	sprintf( szCommand, "SELECT * FROM starting_locations WHERE LocID=2;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
	
	iCol = 3;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, readbuff, sizeof( readbuff ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_MarketLoc.m_flX, sizeof( m_MarketLoc.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_MarketLoc.m_flY, sizeof( m_MarketLoc.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_MarketLoc.m_flZ, sizeof( m_MarketLoc.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_MarketLoc.m_flA, sizeof( m_MarketLoc.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_MarketLoc.m_flB, sizeof( m_MarketLoc.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_MarketLoc.m_flC, sizeof( m_MarketLoc.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &m_MarketLoc.m_flW, sizeof( m_MarketLoc.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	retcode = SQLFetch( cDatabase::m_hStmt );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	sscanf(readbuff,"%08x",&m_MarketLoc.m_dwLandBlock);
	sscanf(dwPosX,"%08x",&m_MarketLoc.m_flX);
	sscanf(dwPosY,"%08x",&m_MarketLoc.m_flY);
	sscanf(dwPosZ,"%08x",&m_MarketLoc.m_flZ);
	sscanf(dwOrientW,"%08x",&m_MarketLoc.m_flA);
	sscanf(dwOrientX,"%08x",&m_MarketLoc.m_flB);
	sscanf(dwOrientY,"%08x",&m_MarketLoc.m_flC);
	sscanf(dwOrientZ,"%08x",&m_MarketLoc.m_flW);

	UpdateConsole( " Marketplace location loaded.\r\n" );
}

/**
 *	Obtains the locations used by the TeleTown command from the database.
 */
void cMasterServer::LoadTeleTownList( )
{
	m_TeleTownList.clear();

	cTeleTownList	cTL;
	char			szCommand[200];
	RETCODE			retcode;
	char			szName[75];
	char			dwLandblock[9];
	char			dwPosX[9];
	char			dwPosY[9];
	char			dwPosZ[9];
	char			dwOrientW[9];
	char			dwOrientX[9];
	char			dwOrientY[9];
	char			dwOrientZ[9];
	
	sprintf( szCommand, "SELECT * FROM tele_locations;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );

	int iCol = 3;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szName, sizeof( szName ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
		sscanf(dwLandblock,"%08x",&cTL.m_dwLandblock);
		sscanf(dwPosX,"%08x",&cTL.m_flPosX);
		sscanf(dwPosY,"%08x",&cTL.m_flPosY);
		sscanf(dwPosZ,"%08x",&cTL.m_flPosZ);
		sscanf(dwOrientW,"%08x",&cTL.m_flOrientW);
		sscanf(dwOrientX,"%08x",&cTL.m_flOrientX);
		sscanf(dwOrientY,"%08x",&cTL.m_flOrientY);
		sscanf(dwOrientZ,"%08x",&cTL.m_flOrientZ);
		
		cTL.m_teleString = szName;
		m_TeleTownList.push_back( cTL );
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

	UpdateConsole( " TeleTown locations loaded.\r\n" );
}

/**
 *	Loads currently existing allegiances from the database.
 *
 *	This data is updated as allegiances are altered in-game.
 *	The data is used to populate allegiance information sent to clients.
 */
void cMasterServer::LoadAllegiances( )
{
	m_AllegianceList.clear();

	char		szCommand[200];
	RETCODE		retcode;
	
	DWORD			AllegianceID;
	char			szName[40];
	DWORD			LeaderGUID;
	char			MOTD[255];
	unsigned long	CreationDate;
	
	sprintf( szCommand, "SELECT * FROM allegiance;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );

	int iCol = 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &AllegianceID, sizeof( DWORD ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szName, sizeof( szName ), NULL );					CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &LeaderGUID, sizeof( DWORD ), NULL );				CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, MOTD, sizeof( MOTD ), NULL );						CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &CreationDate, sizeof( unsigned long ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		cAllegiance* aAllegiance = new cAllegiance();
		aAllegiance->LoadAllegiance(AllegianceID,LeaderGUID,szName,MOTD);
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

	for ( std::vector<cAllegiance*>::iterator itAllegiance = m_AllegianceList.begin() ; itAllegiance != m_AllegianceList.end() ; ++itAllegiance )
	{
		cAllegiance::LoadMembersFromDB(*itAllegiance);
	}

	UpdateConsole( " Allegiances loaded.\r\n" );
}

/**
 *	Loads pre-defined magic models from database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadSpellModels( )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			szMessage[155];
	DWORD			dwModelCount;
	DWORD			dwNumModels;
	char			ModelName[75];
	char			szFlags1[9];
	WORD			wPortalMode;
	WORD			wModel;
	WORD			wIcon;
	char			szParticle[5];
	char			szSoundSet[5];
	char			szModelNumber[5];
	float			flScale;
	DWORD			dwMedGrey;
	DWORD			dwBlueGrey;
	WORD			wSpellAss;
	DWORD			dwSpellID;

	// Start the loading

	sprintf( szCommand, "SELECT MAX(dwSpellID), COUNT(dwSpellID) FROM spellmodels;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwModelCount, sizeof( dwModelCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwNumModels, sizeof( dwNumModels ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwModelCount = 0;
		dwNumModels = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMessage, " Loading %d spell models ... ",dwNumModels );
	UpdateConsole((char *)szMessage);

	// Now all that data needs to be loaded
	sprintf( szCommand, "SELECT * FROM spellmodels ORDER BY ID;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 1;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelName, sizeof( ModelName ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// dwFlags1		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szFlags1, sizeof( szFlags1 ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT,&wPortalMode, sizeof( wPortalMode ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// Animconfig, Soundset
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szSoundSet, sizeof( szSoundSet ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szParticle, sizeof( szParticle ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// dwModelNumber
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szModelNumber, sizeof( szModelNumber ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flScale, sizeof( flScale ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( wModel ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( wIcon ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMedGrey, sizeof( dwMedGrey ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwBlueGrey, sizeof( dwBlueGrey ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSpellAss, sizeof( wSpellAss ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwSpellID, sizeof( dwSpellID ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		cMagicModels* pcModel = new cMagicModels( dwSpellID );

		pcModel->m_strName.assign(ModelName);
		
		pcModel->m_wPortalMode		= wPortalMode;

		pcModel->m_wModel			= wModel;
		pcModel->m_wIcon			= wIcon;

		sscanf(szParticle,"%08x",&pcModel->m_wParticle);
		sscanf(szSoundSet,"%08x",&pcModel->m_wSoundSet);
		sscanf(szModelNumber,"%08x",&pcModel->m_dwModelNumber);

		pcModel->m_flScale			= flScale;
		pcModel->m_dwMedGrey		= dwMedGrey;
		pcModel->m_dwBlueGrey		= dwBlueGrey;

		pcModel->m_wAssociatedSpell	= wSpellAss;
		pcModel->m_dwSpellID		= dwSpellID;
		sscanf(szFlags1,"%08x",&pcModel->m_dwFlags1);

//		UpdateConsole( "+" );
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Loads magic from the database.
 *
 *	This member function loads each spell's characteristics from the database.
 *
 *	TODO: Load from the 0x0E00000E table in the portal.dat instead.
 */
void cMasterServer::LoadSpells( )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			szMessage[155];
	DWORD			dwSpellMax;
	DWORD			dwSpellCount;

	char			SpellName[150];
	char			Desc[255];
	char			School[50];
	char			szIconID[50];
	DWORD			dwEffect;
	char			szResearchable[5];
	DWORD			dwManaCost;
	float			flUnkFloat1;
	float			flUnkFloat2;
	DWORD			dwDifficulty;
	DWORD			dwEconomy;
	char			szVersion[50];
	float			flSpeed;
	DWORD			dwType;
	DWORD			dwSecondID;
	DWORD			dwDuration;
	DWORD			dwUnkDouble;
	char			szComponents[255];
	char			szFiveUnkDoubles[255];
	DWORD			dwSpellID;
	
	// Start the Loading

	sprintf( szCommand, "SELECT MAX(ID), COUNT(ID) FROM spells;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwSpellMax, sizeof( dwSpellMax ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwSpellCount, sizeof( dwSpellCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwSpellMax = 0;
		dwSpellCount = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMessage, " Loading %d spells ... ",dwSpellCount );
	UpdateConsole((char *)szMessage);

	// Now all that Data needs to be loaded
	sprintf( szCommand, "SELECT * FROM spells ORDER BY ID;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 1;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwSpellID, sizeof( dwSpellID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, SpellName, sizeof( SpellName ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Desc, sizeof( Desc ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, School, sizeof( School ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szIconID, sizeof( szIconID ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwEffect, sizeof( dwEffect ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szResearchable, sizeof( szResearchable ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwManaCost, sizeof( dwManaCost ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flUnkFloat1, sizeof( flUnkFloat1 ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flUnkFloat2, sizeof( flUnkFloat2 ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwDifficulty, sizeof( dwDifficulty ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwEconomy, sizeof( dwEconomy ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szVersion, sizeof( szVersion ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flSpeed, sizeof( flSpeed ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwType, sizeof( dwType ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwSecondID, sizeof( dwSecondID ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwDuration, sizeof( dwDuration ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnkDouble, sizeof( dwUnkDouble ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szComponents, sizeof( szComponents ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szFiveUnkDoubles, sizeof( szFiveUnkDoubles ), NULL );CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		cSpell* pcSpell = new cSpell( dwSpellID );

		pcSpell->m_strName.assign(SpellName);
		pcSpell->m_strDesc.assign(Desc);
		pcSpell->m_strSchool.assign(School);
		
		sscanf(szIconID,"%x",&pcSpell->m_dwIconID);
		
		pcSpell->m_dwEffect			= dwEffect;

		if ( strcmp(szResearchable, "Yes") == 0)
			pcSpell->m_fResearchable = true;
		else
			pcSpell->m_fResearchable = false;

		pcSpell->m_dwManaCost		= dwManaCost;
		pcSpell->m_flUnkFloat1		= flUnkFloat1;
		pcSpell->m_flUnkFloat2		= flUnkFloat2;
		pcSpell->m_dwDifficulty		= dwDifficulty;
		pcSpell->m_fEconomy			= dwEconomy;
		pcSpell->m_flSpeed			= flSpeed;
		pcSpell->m_dwType			= dwType;
		pcSpell->m_iDuration		= dwDuration;

		sscanf(szComponents,"%08x %08x %08x %08x %08x %08x %08x %08x %08x",&pcSpell->m_dwComponents[0],&pcSpell->m_dwComponents[1],&pcSpell->m_dwComponents[2],&pcSpell->m_dwComponents[3],&pcSpell->m_dwComponents[4],&pcSpell->m_dwComponents[5],&pcSpell->m_dwComponents[6],&pcSpell->m_dwComponents[7],&pcSpell->m_dwComponents[8],&pcSpell->m_dwComponents[9]);

		DWORD iComp = pcSpell->m_dwComponents[0] & 0xF;

		if (pcSpell->m_dwComponents[5] == 0)
		{
			pcSpell->m_dwLevel = 1;
		}
		else if (pcSpell->m_dwComponents[6] == 0)
		{
			pcSpell->m_dwLevel = 2;
		}
		else if (pcSpell->m_dwComponents[7] == 0)
		{
			if (iComp == 0x0000000A)
			{
				pcSpell->m_dwLevel = 3;
			}
			else if (iComp == 0x00000008)
			{
				pcSpell->m_dwLevel = 4;
			}
		}
		else if (iComp == 0x00000009)
		{
			pcSpell->m_dwLevel = 5;
		}
		else if (iComp == 0x0000000D)
		{
			pcSpell->m_dwLevel = 6;
		}

		sscanf(szFiveUnkDoubles,"%08x",&pcSpell->m_dwEffectAnim);

//		UpdateConsole( "+" );
	}
	
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Loads magic components from the database.
 *
 *	This member function loads each spell component's characteristics from the database.
 *
 *	TODO: Load from the 0x0E00000F table in the portal.dat instead.
 */
void cMasterServer::LoadSpellComps( )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			szMessage[155];
	DWORD			dwCompMax;
	DWORD			dwCompCount;

	DWORD			dwCompID;
	char			CompName[150];
	char			Type[50];
	char			Words[50];
	char			szIconID[50];
	float			flChargeID;
	float			flBurnRate;
	char			szAnimID[50];

	// Start the Loading

	sprintf( szCommand, "SELECT MAX(ID), COUNT(ID) FROM spellcomps;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwCompMax, sizeof( dwCompMax ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwCompCount, sizeof( dwCompCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwCompMax = 0;
		dwCompCount = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMessage, " Loading %d spell components ... ",dwCompCount );
	UpdateConsole((char *)szMessage);

	// Now all that Data needs to be loaded
	sprintf( szCommand, "SELECT * FROM spellcomps ORDER BY ID;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 1;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwCompID, sizeof( dwCompID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, CompName, sizeof( CompName ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Type, sizeof( Type ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szIconID, sizeof( szIconID ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szAnimID, sizeof( szAnimID ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flChargeID, sizeof( flChargeID ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Words, sizeof( Words ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flBurnRate, sizeof( flBurnRate ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		cSpellComp* pcComp = new cSpellComp( dwCompID );

		pcComp->m_strName.assign(CompName);
		pcComp->m_strType.assign(Type);
		pcComp->m_strWords.assign(Words);
		
		sscanf(szIconID,"%x",&pcComp->m_dwIconID);
		sscanf(szAnimID,"%8x",&pcComp->m_dwAnimID);
		
		pcComp->m_flChargeID		= flChargeID;
		pcComp->m_flBurnRate		= flBurnRate;

//		UpdateConsole( "+" );
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Loads the data for the specific item.
 *
 *	This member function initializes the model information for the given item.
 *
 *	@param *pcItem - A pointer to the item to load.
 */
void cMasterServer::LoadItem( cObject *pcItem )
{
   RETCODE   retcode;
   char   szCommand[512];
   char   readbuff01[5];
   char   readbuff02[3];
   char   readbuff03[3];

   BYTE    bWearVectorCount; 

   sprintf( szCommand, "SELECT * FROM avatar_clothing_palettes WHERE ObjectGUID = %lu;",pcItem->GetGUID() );

   retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );                              CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
   retcode = SQLExecute( cDatabase::m_hStmt );                                                   CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
   
   int iCol = 3;
   retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bWearVectorCount, sizeof( BYTE ), NULL );   
   retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff01 , sizeof( readbuff01 ), NULL );      
   retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff02 , sizeof( readbuff02 ), NULL );      
   retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff03 , sizeof( readbuff03 ), NULL );      

   pcItem->m_bWearPaletteChange = 0;
   for ( int palCount = 0 ; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS); ++palCount )
   {
      sscanf(readbuff01,"%04x",&pcItem->m_WearVectorPal[palCount].m_wNewPalette);
      sscanf(readbuff02,"%02x",&pcItem->m_WearVectorPal[palCount].m_ucOffset);
      sscanf(readbuff03,"%02x",&pcItem->m_WearVectorPal[palCount].m_ucLength);

      pcItem->m_bWearPaletteChange = pcItem->m_bWearPaletteChange + 1;
   }

   retcode = SQLCloseCursor( cDatabase::m_hStmt );            CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
   retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

   cItemModels *pcModel = cItemModels::FindModel(pcItem->GetItemModelID());
   if (pcModel->m_PortalLinker != 0)
   {
      pcItem->m_bPaletteChange = 0;
      pcModel->m_bModelChange = 0;
      pcModel->m_bTextureChange = 0;
      cPortalDat::LoadItemModel(pcItem, DWORD(0x02000000 | pcModel->m_dwModelNumber),pcItem->m_intColor);
   }
}


/**
 *	Loads predefined item models from the database.
 *
 *	An item model comprises the general characteristics of a given item.
 *
 *	Note:  Used for items.
 *
 *	TODO: Load from the 0x10000000 entries in the portal.dat instead.
 *
 *	Author: Cubem0j0
 */
void cMasterServer::LoadItemModels( )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			szMessage[155];
	DWORD			dwModelCount;
	DWORD			dwNumModels;

	char			ModelName[75];
	char			ModelDescription[512];
	char			PortalLinker[9];
	int				item_type;
	DWORD			dwModelID;
	BYTE			bPaletteChange;
	sPaletteChange	vPalettes[255];
	BYTE			bTextureChange;

	sTextureChange	vTextures[255];
	BYTE			bModelChange;

	sModelChange	vModels[255];

	char			szFlags1[9];
	char			szFlags2[9];
	char			szObjFlags1[9];
	char			szObjFlags2[9];
	WORD			wPortalMode;
//	char			szModel[5];
//	char			szIcon[5];
	WORD			wModel;
	WORD			wIcon;
	char			szSoundSet[5];
	char			szwUnknown_1[5];
	int				bIsContainer;
	int				bIsClothing;

	DWORD			dwUnknown_Blue;
	char			szModelNumber[5];
	float			flScale;
	DWORD			dwUnknown_LightGrey;
	DWORD			dwTrio1[3];
	DWORD			dwMedGrey;
	DWORD			dwBlueGrey;
	WORD			wSeagreen8;
	
	DWORD			dwUnknownCount;
	DWORD			dwUnknown_v2;
	DWORD			dwUnknown_v6;

	DWORD			dwValue;
	char			szUseableOn[9];
//	float			fApproachDistance;
	DWORD			dwIconHighlight;
	WORD			wAmmoType;
	BYTE			bWieldType;
	WORD			wUses;
	WORD			wUseLimit;
	WORD			wStack;
	WORD			wStackLimit;
//	DWORD			dwContainerID;
	DWORD			dwVitalID;
	char			szEquipPossible[9];
	char			szEquipActual[9];
	char			szCoverage[9];
	float			fWorkmanship;
	WORD			wBurden;
	//DWORD			dwSpellID;
	WORD			wSpellID;
	DWORD			dwOwner;
	WORD			wHooks;
	DWORD			dwMaterialType;
	DWORD			dwQuestItemID;
	int				vital_affect;
	int				IsUAWeapon;

	DWORD			dwArmorLevel;
	float			fProt_S;
	float			fProt_P;
	float			fProt_B;
	float			fProt_F;
	float			fProt_C;
	float			fProt_A;
	float			fProt_E;

	DWORD	dwWeaponDamage;
	DWORD	dwWeaponSpeed;
	DWORD	dwWeaponSkill;
	DWORD	dwDamageType;

	double	dWeaponVariance;
	double	dWeaponModifier;
	double	dWeaponPower;
	double	dWeaponAttack;

	char szTitle[100];	// = "Asherons Call";
	char szAuthor[100];		// = "prewritten";
	char szComment[100];		// = "Welcome To Asheron's Call";
	char szCommentAuthorName[100];		// = "CommentAuthorName";
	
	DWORD dwUsedPages;
	DWORD dwContentPages;
	DWORD dwTotalPages;

	char szPageOneText[2048];
	char szPageTwoText[2048];
	char szPageThreeText[2048];
	//Start the loading

	sprintf( szCommand, "SELECT MAX(dwLinker),COUNT(dwLinker) FROM items_templates;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwModelCount, sizeof( dwModelCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwNumModels, sizeof( dwNumModels ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwModelCount = 0;
		dwNumModels = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMessage, " Loading %d pre-defined item models ... ",dwNumModels );
	UpdateConsole((char *)szMessage);

	//Now all that data needs to be loaded
//	sprintf( szCommand, "SELECT ID,quest_item_id,Name,Description,ItemType,bPalette,bTexture,bModel,dwFlags1,wPortalMode,wUnknown_1,wModel,wIcon,SoundSet,dwUnknown_Blue,dwModelNumber,flScale,dwUnknown_LightGrey,dwTrio1_1,dwTrio1_2,dwTrio1_3,dwMedGrey,dwBlueGrey,dwSeagreen8,dwUnknown_v2,dwUnknown_v6,dwUnkCount,dwLinker,dwFlags2,dwObjectFlags1,dwObjectFlags2,dwValue,dwUseableOn,dwIconHighlight,wAmmoType,bWieldType,wUses,wUseLimit,wStack,wStackLimit,dwVitalID,dwEquipPoss,dwEquipAct,dwCoverage,fWorkmanship,wBurden,dwSpellID,dwOwner,wHooks,dwMaterial,vital_effect,dwArmorLevel,fProt_Slashing,fProt_Piercing,fProt_Bludgeon,fProt_Fire,fProt_Cold,fProt_Acid,fProt_Electric,dwWeaponDamage,dwWeaponSpeed,dwWeaponSkill,dwDamageType,dWeaponVariance,dWeaponModifier,dWeaponPower,ddWeaponAttack FROM items_templates ORDER BY ID;" );
	sprintf( szCommand, "SELECT * FROM items_templates ORDER BY ID;" );
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	int iCol = 2;

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwQuestItemID, sizeof( dwQuestItemID ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelName, sizeof( ModelName ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelDescription, sizeof( ModelDescription ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &item_type, sizeof( item_type), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, PortalLinker, sizeof( PortalLinker ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bPaletteChange, sizeof( BYTE), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bTextureChange, sizeof( BYTE ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bModelChange, sizeof( BYTE ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// dwFlags1
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szFlags1, sizeof( szFlags1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wPortalMode, sizeof( wPortalMode ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szwUnknown_1, sizeof( szwUnknown_1 ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bIsContainer, sizeof( INT ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bIsClothing, sizeof( INT ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &IsUAWeapon, sizeof( INT ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( wModel ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( wIcon ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szModel, sizeof( szModel ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szIcon, sizeof( szIcon ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	//Animconfig, Soundset
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szAnimConfig, sizeof( szAnimConfig ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szSoundSet, sizeof( szSoundSet ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_Blue, sizeof( dwUnknown_Blue ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//dwModelNumber		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szModelNumber, sizeof( szModelNumber ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flScale, sizeof( flScale ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_LightGrey, sizeof( dwUnknown_LightGrey ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[0], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[1], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[2], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMedGrey, sizeof( dwMedGrey ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwBlueGrey, sizeof( dwBlueGrey ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSeagreen8, sizeof( wSeagreen8 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_v2, sizeof( dwUnknown_v2 ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_v6, sizeof( dwUnknown_v6 ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknownCount, sizeof( dwUnknownCount ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelID, sizeof( dwModelID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szFlags2, sizeof( szFlags2 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szObjFlags1, sizeof( szObjFlags1 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szObjFlags2, sizeof( szObjFlags2 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwValue, sizeof( dwValue ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szUseableOn, sizeof( szUseableOn ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fApproachDistance, sizeof( fApproachDistance ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwIconHighlight, sizeof( dwIconHighlight ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wAmmoType, sizeof( wAmmoType ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bWieldType, sizeof( BYTE ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wUses, sizeof( wUses ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wUseLimit, sizeof( wUseLimit ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wStack, sizeof( wStack ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wStackLimit, sizeof( wStackLimit ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwContainerID, sizeof( dwContainerID ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwVitalID, sizeof( dwVitalID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szEquipPossible, sizeof( szEquipPossible ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szEquipActual, sizeof( szEquipActual ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szCoverage, sizeof( szCoverage ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fWorkmanship, sizeof( fWorkmanship ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wBurden, sizeof( wBurden ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSpellID, sizeof( wSpellID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwOwner, sizeof( dwOwner ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wHooks, sizeof( wHooks ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMaterialType, sizeof( dwMaterialType ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &vital_affect, sizeof( vital_affect ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwArmorLevel, sizeof( dwArmorLevel ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fProt_S, sizeof( fProt_S ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fProt_P, sizeof( fProt_P ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fProt_B, sizeof( fProt_B ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fProt_F, sizeof( fProt_F ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fProt_C, sizeof( fProt_C ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fProt_A, sizeof( fProt_A ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &fProt_E, sizeof( fProt_E ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwWeaponDamage, sizeof( dwWeaponDamage ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwWeaponSpeed, sizeof( dwWeaponSpeed ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwWeaponSkill, sizeof( dwWeaponSkill ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwDamageType, sizeof( dwDamageType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &dWeaponVariance, sizeof( dWeaponVariance ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &dWeaponModifier, sizeof( dWeaponModifier ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &dWeaponPower, sizeof( dWeaponPower ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &dWeaponAttack, sizeof( dWeaponAttack ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS ; ++i )
	{
		cItemModels* pcModel = new cItemModels( dwModelID );
		
		pcModel->m_dwQuestItemID		= dwQuestItemID;
		pcModel->m_strName.assign(ModelName);
		pcModel->m_strDescription.assign(ModelDescription);
		sscanf(PortalLinker,"%08x",&pcModel->m_PortalLinker);
		pcModel->m_ItemType				= item_type;
		pcModel->m_bPaletteChange		= bPaletteChange;
		pcModel->m_wPaletteVector		= dwModelID;
		pcModel->m_bTextureChange		= bTextureChange;
		pcModel->m_wTextureVector		= dwModelID;
		pcModel->m_bModelChange			= bModelChange;
		pcModel->m_wModelVector			= dwModelID;
		pcModel->m_wWearPaletteVector	= dwModelID;
		pcModel->m_wWearTextureVector	= dwModelID;
		pcModel->m_wWearModelVector		= dwModelID;
		
		sscanf(szFlags1,"%08x",&pcModel->m_dwFlags1);
		
		pcModel->m_wPortalMode		= wPortalMode;

		sscanf(szwUnknown_1, "%04x",&pcModel->m_wUnknown_1);
		pcModel->m_isContainer		= bIsContainer;
		pcModel->m_isClothing		= bIsClothing;
		pcModel->m_isUAWeapon		= IsUAWeapon;
		pcModel->m_wModel			= wModel;
		pcModel->m_wIcon			= wIcon;
//		sscanf(szModel,"%04x",&pcModel->m_wModel);
//		sscanf(szIcon,"%04x",&pcModel->m_wIcon);

		sscanf(szSoundSet,"%08x",&pcModel->m_wSoundSet);
		
		pcModel->m_dwUnknown_Blue	= dwUnknown_Blue;

		sscanf(szModelNumber,"%08x",&pcModel->m_dwModelNumber);

		pcModel->m_flScale			= flScale;
		pcModel->m_dwUnknown_LightGrey = dwUnknown_LightGrey;
		pcModel->m_dwTrio1[0]		= dwTrio1[0];
		pcModel->m_dwTrio1[1]		= dwTrio1[1];
		pcModel->m_dwTrio1[2]		= dwTrio1[2];
		pcModel->m_dwMedGrey		= dwMedGrey;
		pcModel->m_dwBlueGrey		= dwBlueGrey;
		pcModel->m_wSeagreen8		= wSeagreen8;
		pcModel->m_dwUnknown_v2		= dwUnknown_v2;
		pcModel->m_dwUnknown_v6		= dwUnknown_v6;
		pcModel->m_dwUnknownCount	= dwUnknownCount;

		sscanf(szFlags2, "%08x",&pcModel->m_dwFlags2);
		sscanf(szObjFlags1, "%08x",&pcModel->m_dwObjectFlags1);
		sscanf(szObjFlags2, "%08x",&pcModel->m_dwObjectFlags2);

		pcModel->m_dwValue			= dwValue;
		sscanf(szUseableOn, "%08x",&pcModel->m_dwUseableOn);
		//pcModel->m_fApproachDistance = fApproachDistance;
		pcModel->m_dwIconHighlight	= dwIconHighlight;
		pcModel->m_wAmmoType		= wAmmoType;
		pcModel->m_bWieldType		= bWieldType;
		pcModel->m_wUses			= wUses;
		pcModel->m_wUseLimit		= wUseLimit;
		pcModel->m_wStack			= wStack;
		pcModel->m_wStackLimit		= wStackLimit;
		//pcModel->m_dwContainerID	= dwContainerID;
		pcModel->m_dwVitalID		= dwVitalID;

		sscanf(szEquipPossible, "%08x",&pcModel->m_dwEquipPossible);
		sscanf(szEquipActual, "%08x",&pcModel->m_dwEquipActual);
		sscanf(szCoverage, "%08x",&pcModel->m_dwCoverage);

		pcModel->m_fWorkmanship		= fWorkmanship;
		pcModel->m_wBurden			= wBurden;
		pcModel->m_wSpellID			= wSpellID;
		pcModel->m_dwOwner			= dwOwner;
		pcModel->m_wHooks			= wHooks;
		pcModel->m_dwMaterialType   = dwMaterialType;
		pcModel->m_vital_affect		= vital_affect;

		pcModel->m_dwArmor_Level	= dwArmorLevel;
		pcModel->m_fProt_Slashing	= fProt_S;
		pcModel->m_fProt_Piercing	= fProt_P;
		pcModel->m_fProt_Bludgeon	= fProt_B;
		pcModel->m_fProt_Fire		= fProt_F;
		pcModel->m_fProt_Cold		= fProt_C;
		pcModel->m_fProt_Acid		= fProt_A;
		pcModel->m_fProt_Electric	= fProt_E;

		pcModel->m_dwWeaponDamage	= dwWeaponDamage;
		pcModel->m_dwWeaponSpeed	= dwWeaponSpeed;
		pcModel->m_dwWeaponSkill	= dwWeaponSkill;
		pcModel->m_dwDamageType		= dwDamageType;

		pcModel->m_dWeaponVariance	= dWeaponVariance;
		pcModel->m_dWeaponModifier	= dWeaponModifier;
		pcModel->m_dWeaponPower		= dWeaponPower;
		pcModel->m_dWeaponAttack	= dWeaponAttack;
	
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	/* k109:  Load book data	*/

	for ( DWORD d = 0; d < dwModelCount+1; d++)
	{
		cItemModels *pcModel = cItemModels::Hash_Find( d );
		if(pcModel)
		{
			if(pcModel->m_ItemType == 4)
			{
				sprintf( szCommand, "SELECT * FROM items_books WHERE dwLinker = %d;",pcModel->m_dwModelID );
				retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				
				int iCol = 5;
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szTitle, sizeof( szTitle ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szAuthor, sizeof( szAuthor ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szComment, sizeof( szComment ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szCommentAuthorName, sizeof( szCommentAuthorName ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUsedPages, sizeof( dwUsedPages ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwContentPages, sizeof( dwContentPages ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwTotalPages, sizeof( dwTotalPages ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szPageOneText, sizeof( szPageOneText ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szPageTwoText, sizeof( szPageTwoText ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szPageThreeText, sizeof( szPageThreeText ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

				for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
				{
					pcModel->m_Title.assign(szTitle);
					pcModel->m_Author.assign(szAuthor);
					pcModel->m_Comment.assign(szComment);
					pcModel->m_CommentAuthor.assign(szCommentAuthorName);

					pcModel->m_UsedPages = dwUsedPages;
					pcModel->m_ContentPages = dwContentPages;
					pcModel->m_TotalPages = dwTotalPages;

					pcModel->m_Pages[0].textPages = szPageOneText;
					pcModel->m_Pages[1].textPages = szPageTwoText;
					pcModel->m_Pages[2].textPages = szPageThreeText;
					//pcModel->m_Page1.assign(szPageOneText);
					//pcModel->m_Page2.assign(szPageTwoText);
					//pcModel->m_Page3.assign(szPageThreeText);
				}

				retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
			}
		}
	}
	
	//===============================================================
	// Load all of the Vector Data for Each Model ID
	// 
	//===============================================================
	for ( d = 0; d < dwModelCount+1; d++)
	{
		cItemModels *pcModel = cItemModels::Hash_Find( d );
		if(pcModel) 
		{
			sprintf( szCommand, "SELECT * FROM items_vector_palettes WHERE ModelVector=%d ORDER BY VectorCount;", pcModel->m_wPaletteVector);

			// Load Palette vector into array

			// Define Temp Variables
			BYTE    bVectorCount; 
			char	readbuff01[5];
			char	readbuff02[3];
			char	readbuff03[3];

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );	
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff01 , sizeof( readbuff01 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff02 , sizeof( readbuff02 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff03 , sizeof( readbuff03 ), NULL );		

			for ( int intPcount = 0 ; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) && (intPcount < pcModel->m_bPaletteChange +1); ++intPcount )
			{
				// Scan the text into hexadecimal
				sscanf(readbuff01,"%04x",&pcModel->m_vectorPal[intPcount].m_wNewPalette);
				sscanf(readbuff02,"%02x",&pcModel->m_vectorPal[intPcount].m_ucOffset);
				sscanf(readbuff03,"%02x",&pcModel->m_vectorPal[intPcount].m_ucLength);
			}
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			// Load Texture Vector into Array

			// Define Temp Variables
			char	readbuff04[3];
			char	readbuff05[5];
			char	readbuff06[5];

			sprintf( szCommand, "SELECT * FROM items_vector_textures WHERE ModelVector=%d ORDER BY VectorCount;",pcModel->m_wTextureVector);
		
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff04, sizeof( readbuff04 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff05, sizeof( readbuff05 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff06 , sizeof( readbuff06 ), NULL );	

			for ( int intT = 0; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) && (intT < pcModel->m_bTextureChange + 1); ++intT )
			{
				sscanf(readbuff04,"%02x",&pcModel->m_vectorTex[intT].m_bModelIndex);
				sscanf(readbuff05,"%04x",&pcModel->m_vectorTex[intT].m_wOldTexture);
				sscanf(readbuff06,"%04x",&pcModel->m_vectorTex[intT].m_wNewTexture);
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			//  Load Model Vectors into Array

			// Define Temp Variables
			char	readbuff07[3];
			char	readbuff08[5];

			sprintf( szCommand, "SELECT * FROM items_vector_models WHERE ModelVector=%d ORDER BY VectorCount;", pcModel->m_wModelVector);

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff07, sizeof( readbuff07 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff08 , sizeof( readbuff08 ), NULL );
				
			
			for ( int intM = 0; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) && (intM < pcModel->m_bModelChange + 1); ++intM )
			{
				sscanf(readbuff07,"%02x",&pcModel->m_vectorMod[intM].m_bModelIndex);
				sscanf(readbuff08,"%04x",&pcModel->m_vectorMod[intM].m_wNewModel);	
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		}
	}

	//===============================================================
	// Load all of the Avatar Model Vector Data for Each Model ID
	// 
	//===============================================================
	for ( d = 0; d < dwModelCount+1; d++)
	{
		cItemModels *pcModel = cItemModels::Hash_Find( d );

		if(pcModel && pcModel->m_isClothing == 1) 
		{	
			if (pcModel->m_PortalLinker != 0)
			{
				pcModel->m_bWearModelChange = 0;
				pcModel->m_bWearTextureChange = 0;
				pcModel->m_clothingModelLoaded = false;
			} else {
				pcModel->m_clothingModelLoaded = true;
			// Define Temp Variables
			sprintf( szCommand, "SELECT * FROM avatar_clothing_palettes WHERE ObjectGUID=%d ORDER BY VectorCount;", pcModel->m_wWearPaletteVector);

			// Load Palette vector into array

			// Define Temp Variables
			BYTE    bWearVectorCount; 
			char	readbuff09[5];
			char	readbuff10[3];
			char	readbuff11[3];

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bWearVectorCount, sizeof( BYTE ), NULL );	
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff09 , sizeof( readbuff09 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff10 , sizeof( readbuff10 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff11 , sizeof( readbuff11 ), NULL );		

			pcModel->m_bWearPaletteChange = 0;
			for ( int intPcount = 0 ; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS); ++intPcount )
			{
				sscanf(readbuff09,"%04x",&pcModel->m_WearVectorPal[intPcount].m_wNewPalette);
				sscanf(readbuff10,"%02x",&pcModel->m_WearVectorPal[intPcount].m_ucOffset);
				sscanf(readbuff11,"%02x",&pcModel->m_WearVectorPal[intPcount].m_ucLength);

				pcModel->m_bWearPaletteChange = pcModel->m_bWearPaletteChange + 1;
			}
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			// Load Texture Vector into Array
			char	readbuff12[3];
			char	readbuff13[5];
			char	readbuff14[5];

			sprintf( szCommand, "SELECT * FROM avatar_clothing_textures WHERE Linker=%d ORDER BY VectorCount;",pcModel->m_wWearTextureVector);
		
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bWearVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff12, sizeof( readbuff12 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff13, sizeof( readbuff13 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff14 , sizeof( readbuff14 ), NULL );	

			pcModel->m_bWearTextureChange = 0;
			for ( int intT = 0; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS); ++intT )
			{
				sscanf(readbuff12,"%02x",&pcModel->m_WearVectorTex[intT].m_bModelIndex);
				sscanf(readbuff13,"%04x",&pcModel->m_WearVectorTex[intT].m_wOldTexture);
				sscanf(readbuff14,"%04x",&pcModel->m_WearVectorTex[intT].m_wNewTexture);

				pcModel->m_bWearTextureChange = pcModel->m_bWearTextureChange + 1;
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			//  Load Model Vectors into Array

			// Define Temp Variables
			char	readbuff15[3];
			char	readbuff16[5];

			sprintf( szCommand, "SELECT * FROM avatar_clothing_models WHERE Linker=%d ORDER BY VectorCount;", pcModel->m_wWearModelVector);

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bWearVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff15, sizeof( readbuff15 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff16 , sizeof( readbuff16 ), NULL );
				
			pcModel->m_bWearModelChange = 0;
			for ( int intM = 0; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS); ++intM )
			{
				sscanf(readbuff15,"%02x",&pcModel->m_WearVectorMod[intM].m_bModelIndex);
				sscanf(readbuff16,"%04x",&pcModel->m_WearVectorMod[intM].m_wNewModel);

				pcModel->m_bWearModelChange = pcModel->m_bWearModelChange + 1;
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

//			UpdateConsole( "+" );
			}
		}
	}
}

/**
 *	Loads pre-defined world object models from the database.
 *
 *	A world object model comprises the general characteristics of a world object item.
 *
 *	Author: Cubem0j0
 */
void cMasterServer::LoadWorldModels( )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			szMessage[155];
	DWORD			dwModelCount;
	DWORD			dwNumModels;

	char			ModelName[75];
	char			ModelDescription[512];
	char			ModelDescription2[512];
	DWORD			dwModelID;
	BYTE			bPaletteChange;
	sPaletteChange	vPalettes[255];
	BYTE			bTextureChange;

	sTextureChange	vTextures[255];
	BYTE			bModelChange;

	sModelChange	vModels[255];

	char			szFlags1[9];
	char			szFlags2[9];
	char			szObjFlags1[9];
	char			szObjFlags2[9];
	WORD			wPortalMode;
	WORD			wUnknown_1;
	WORD			wModel;
	WORD			wIcon;
	char			szSoundSet[5];
	char			szAnimConfig[5];
	
	DWORD			dwUnknown_Blue;
	char			szModelNumber[5];
	float			flScale;
	DWORD			dwUnknown_LightGrey;
	DWORD			dwTrio1[3];
	DWORD			dwMedGrey;
	DWORD			dwBlueGrey;
	WORD			wSeagreen8;
	
	DWORD			dwUnknownCount;
	char			bInitialAnimation[200];
	DWORD			dwUnknown_v2;
	DWORD			dwUnknown_v6;

	// Start the loading

	sprintf( szCommand, "SELECT MAX(dwLinker),COUNT(dwLinker) FROM worldobjects_templates;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwModelCount, sizeof( dwModelCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwNumModels, sizeof( dwNumModels ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwModelCount = 0;
		dwNumModels = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMessage, " Loading %d pre-defined world object models ... ",dwNumModels );
	UpdateConsole((char *)szMessage);

	// Now all that data needs to be loaded
//	sprintf( szCommand, "SELECT ID,Name,Description,Description2,bPalette,bTexture,bModel,dwFlags1,wPortalMode,wUnknown_1,wModel,wIcon,SoundSet,AnimConfig,dwUnknown_Blue,dwModelNumber,flScale,dwUnknown_LightGrey,dwTrio1_1,dwTrio1_2,dwTrio1_3,dwMedGrey,dwBlueGrey,dwSeagreen8,dwUnknown_v2,dwUnknown_v6,dwUnkCount,unknownBytes,dwLinker,dwFlags2,dwObjectFlags1,dwObjectFlags2 FROM worldobjects_templates ORDER BY ID;" );
	sprintf( szCommand, "SELECT * FROM worldobjects_templates ORDER BY ID;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	int iCol = 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelName, sizeof( ModelName ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelDescription, sizeof( ModelDescription ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelDescription2, sizeof( ModelDescription2 ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bPaletteChange, sizeof( BYTE), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bTextureChange, sizeof( BYTE ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bModelChange, sizeof( BYTE ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// dwFlags1		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szFlags1, sizeof( szFlags1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wPortalMode, sizeof( wPortalMode ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wUnknown_1, sizeof( wUnknown_1 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( wModel ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( wIcon ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// Animconfig, Soundset
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szSoundSet, sizeof( szSoundSet ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szAnimConfig, sizeof( szAnimConfig ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_Blue, sizeof( dwUnknown_Blue ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// dwModelNumber		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szModelNumber, sizeof( szModelNumber ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flScale, sizeof( flScale ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_LightGrey, sizeof( dwUnknown_LightGrey ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[0], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[1], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[2], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMedGrey, sizeof( dwMedGrey ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwBlueGrey, sizeof( dwBlueGrey ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSeagreen8, sizeof( wSeagreen8 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_v2, sizeof( dwUnknown_v2 ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_v6, sizeof( dwUnknown_v6 ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknownCount, sizeof( dwUnknownCount ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &bInitialAnimation, sizeof( bInitialAnimation ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelID, sizeof( dwModelID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szFlags2, sizeof( szFlags2 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR,&szObjFlags1, sizeof( szObjFlags1 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR,&szObjFlags2, sizeof( szObjFlags2 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
//		#pragma warning(disable:4786)

		cWObjectModels* pcModel = new cWObjectModels( dwModelID );
		
		pcModel->m_strName.assign(ModelName);
		pcModel->m_strDescription.assign(ModelDescription);
		pcModel->m_bPaletteChange	= bPaletteChange;
		pcModel->m_wPaletteVector	= dwModelID;
		pcModel->m_bTextureChange	= bTextureChange;
		pcModel->m_wTextureVector	= dwModelID;
		pcModel->m_bModelChange		= bModelChange;
		pcModel->m_wModelVector		= dwModelID;
		
		sscanf(szFlags1,"%08x",&pcModel->m_dwFlags1);
		
		pcModel->m_wPortalMode		= wPortalMode;
		pcModel->m_wUnknown_1		= wUnknown_1;
		pcModel->m_wModel			= wModel;
		pcModel->m_wIcon			= wIcon;

		sscanf(szSoundSet,"%08x",&pcModel->m_wSoundSet);
		sscanf(szAnimConfig,"%08x",&pcModel->m_wAnimConfig);
		
		pcModel->m_dwUnknown_Blue	= dwUnknown_Blue;

		sscanf(szModelNumber,"%08x",&pcModel->m_dwModelNumber);

		pcModel->m_flScale			= flScale;
		pcModel->m_dwUnknown_LightGrey = dwUnknown_LightGrey;
		pcModel->m_dwTrio1[0]		= dwTrio1[0];
		pcModel->m_dwTrio1[1]		= dwTrio1[1];
		pcModel->m_dwTrio1[2]		= dwTrio1[2];
		pcModel->m_dwMedGrey		= dwMedGrey;
		pcModel->m_dwBlueGrey		= dwBlueGrey;
		pcModel->m_wSeagreen8		= wSeagreen8;
		pcModel->m_dwUnknown_v2		= dwUnknown_v2;
		pcModel->m_dwUnknown_v6		= dwUnknown_v6;
		pcModel->m_dwUnknownCount	= dwUnknownCount;

		sscanf(szFlags2, "%08x",&pcModel->m_dwFlags2);
		sscanf(szObjFlags1, "%08x",&pcModel->m_dwObjectFlags1);
		sscanf(szObjFlags2, "%08x",&pcModel->m_dwObjectFlags2);
		
		int b = 0;
		char	cTemp[2];
		for(int i = 0; i < (dwUnknownCount); i++) 
		{
			CopyMemory( &cTemp[0], &bInitialAnimation[b], 2 );
			sscanf(cTemp,"%08x",&pcModel->m_bInitialAnimation[i]);
			b = b + 2;
		}
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	//===============================================================
	// Load all the Vector Data for Each Model ID
	// 
	//===============================================================
	for ( DWORD d = 0; d < dwModelCount+1; d++)
	{
		cWObjectModels *pcModel = cWObjectModels::Hash_Find( d );
		if(pcModel) 
		{
			sprintf( szCommand, "SELECT * FROM worldobjects_vector_palettes WHERE dwLinker=%d ORDER BY VectorCount;", pcModel->m_wPaletteVector);

			// Load Palette vector into array

			// Define Temp Variables
			BYTE    bVectorCount; 
			char	readbuff01[5];
			char	readbuff02[3];
			char	readbuff03[3];

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( bVectorCount ), NULL );	
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff01 , sizeof( readbuff01 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff02 , sizeof( readbuff02 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff03 , sizeof( readbuff03 ), NULL );		

			for ( int intPcount = 0 ; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) && (intPcount < pcModel->m_bPaletteChange +1); ++intPcount )
			{
				// Let's Scan that Text into Hex
				sscanf(readbuff01,"%04x",&pcModel->m_vectorPal[intPcount].m_wNewPalette);
				sscanf(readbuff02,"%02x",&pcModel->m_vectorPal[intPcount].m_ucOffset);
				sscanf(readbuff03,"%02x",&pcModel->m_vectorPal[intPcount].m_ucLength);
			}
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			// Load Texture Vector into Array

			// Define Temp Variables
			char	readbuff04[3];
			char	readbuff05[5];
			char	readbuff06[5];

			sprintf( szCommand, "SELECT * FROM worldobjects_vector_textures WHERE dwLinker=%d ORDER BY VectorCount;",pcModel->m_wTextureVector);
		
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff04, sizeof( readbuff04 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff05, sizeof( readbuff05 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff06 , sizeof( readbuff06 ), NULL );	

			for ( int intT = 0; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) && (intT < pcModel->m_bTextureChange + 1); ++intT )
			{
				// Let's Scan that Text into Hex
				sscanf(readbuff04,"%02x",&pcModel->m_vectorTex[intT].m_bModelIndex);
				sscanf(readbuff05,"%04x",&pcModel->m_vectorTex[intT].m_wOldTexture);
				sscanf(readbuff06,"%04x",&pcModel->m_vectorTex[intT].m_wNewTexture);
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt );			CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			//  Load Model Vectors into Array

			// Define Temp Variables
			char	readbuff07[3];
			char	readbuff08[5];

			sprintf( szCommand, "SELECT * FROM worldobjects_vector_models WHERE dwLinker=%d ORDER BY VectorCount;", pcModel->m_wModelVector);

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
		
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff07, sizeof( readbuff07 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff08 , sizeof( readbuff08 ), NULL );	
			
			for ( int intM = 0; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) && (intM < pcModel->m_bModelChange + 1); ++intM )
			{
				// Scan the Text to HEX
				sscanf(readbuff07,"%02x",&pcModel->m_vectorMod[intM].m_bModelIndex);
				sscanf(readbuff08,"%04x",&pcModel->m_vectorMod[intM].m_wNewModel);		
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

//			UpdateConsole( "+" );
		}
	}
}

/**
 *	Loads merchant signs from the database.
 *
 *	Author: Cubem0j0
 */
void cMasterServer::LoadWorldObjects2( )
{
	RETCODE		retcode;
	char		szCommand[512];
//	char		szItems[150];
	cLocation	locWO;
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	char		Name[100];
	char		Description[255];
//	DWORD		dwItemCount;
//	cLocation	locItem;
	DWORD		dwModelNumber;
	WORD		wModel;
	WORD		wIcon;
	int			iObjectType = 0;
	WORD		wWOcount = 0;
	char		szWorldObjects[150];
//	DWORD		dwModelID;

	sprintf( szCommand, "SELECT COUNT(ID) FROM worldobjects2;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wWOcount, sizeof( wWOcount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		wWOcount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	

	sprintf( szWorldObjects, " Loading %d merchant signs ... ",wWOcount );
	UpdateConsole((char *)szWorldObjects);

	sprintf( szCommand, "SELECT * FROM worldobjects2;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 3;

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Name, sizeof( Name ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Description, sizeof( Description ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelNumber, sizeof( dwModelNumber ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// Landblock Data
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flX, sizeof( &locWO.m_flX ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flY, sizeof( &locWO.m_flY ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flZ, sizeof( &locWO.m_flZ ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flA, sizeof( &locWO.m_flA ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flB, sizeof( &locWO.m_flB ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flC, sizeof( &locWO.m_flC ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flW, sizeof( &locWO.m_flW ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT,&wModel, sizeof( wModel ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( wIcon ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &iObjectType, sizeof( INT ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
		sscanf(dwLandblock,"%08x",&locWO.m_dwLandBlock);
		sscanf(dwPosX,"%08x",&locWO.m_flX);
		sscanf(dwPosY,"%08x",&locWO.m_flY);
		sscanf(dwPosZ,"%08x",&locWO.m_flZ);
		sscanf(dwOrientW,"%08x",&locWO.m_flA);
		sscanf(dwOrientX,"%08x",&locWO.m_flB);
		sscanf(dwOrientY,"%08x",&locWO.m_flC);
		sscanf(dwOrientZ,"%08x",&locWO.m_flW);

		cMerchantSign* aObject = new cMerchantSign(cWorldManager::NewGUID_Object(),locWO,dwModelNumber,1.0,TRUE,wIcon,Name,Description,4,7,TRUE);
		aObject->SetObjectType(iObjectType);
		cWorldManager::AddObject( aObject, TRUE );

//		UpdateConsole( "+" );
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Loads ground spawns from the database
 *
 *	Note:  Use ModelNumber to reference pre-loaded model data.
 *
 *	Author: Cubem0j0
 */
void cMasterServer::LoadGroundSpawns( )
{
	RETCODE		retcode;
	char		szCommand[512];
	char		szItems[150];
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	char		Name[100];
	char		Description[255];
	DWORD		dwItemCount;
	cLocation	locItem;
	DWORD		dwModelNumber;
	int			itemType = 0;
	DWORD		dwQuantity;
	DWORD		dwQuestItemID;

	sprintf( szCommand, "SELECT COUNT(ID) FROM items_instance_ground;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );											CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwItemCount, sizeof( dwItemCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		dwItemCount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szItems, " Loading %d ground spawns ... ",dwItemCount );
	UpdateConsole((char *)szItems);

	sprintf( szCommand, "SELECT * FROM items_instance_ground;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwQuestItemID, sizeof( dwQuestItemID ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Name, sizeof( Name ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Description, sizeof( Description ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelNumber, sizeof( dwModelNumber ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &itemType, sizeof( INT ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwQuantity, sizeof( dwQuantity ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	// Landblock Data
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locItem.m_flX, sizeof( &locItem.m_flX ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locItem.m_flY, sizeof( &locItem.m_flY ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locItem.m_flZ, sizeof( &locItem.m_flZ ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locItem.m_flA, sizeof( &locItem.m_flA ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locItem.m_flB, sizeof( &locItem.m_flB ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locItem.m_flC, sizeof( &locItem.m_flC ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locItem.m_flW, sizeof( &locItem.m_flW ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		sscanf(dwLandblock,"%08x",&locItem.m_dwLandBlock);
		sscanf(dwPosX,"%08x",&locItem.m_flX);
		sscanf(dwPosY,"%08x",&locItem.m_flY);
		sscanf(dwPosZ,"%08x",&locItem.m_flZ);
		sscanf(dwOrientW,"%08x",&locItem.m_flA);
		sscanf(dwOrientX,"%08x",&locItem.m_flB);
		sscanf(dwOrientY,"%08x",&locItem.m_flC);
		sscanf(dwOrientZ,"%08x",&locItem.m_flW);

		//Cubem0j0:  We find the model number so we can get some vars.
		cItemModels *pcModel = cItemModels::FindModel(dwModelNumber);

		switch(itemType)
		{
			//Weapon
			case 1:
			{
				cWeapon* aWeapon = new cWeapon(cWorldManager::NewGUID_Object(),locItem,dwModelNumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->m_dwWeaponDamage,pcModel->m_dwWeaponSpeed,pcModel->m_dwWeaponSkill,pcModel->m_dwDamageType,pcModel->m_dWeaponVariance,pcModel->m_dWeaponModifier,pcModel->m_dWeaponPower,pcModel->m_dWeaponAttack);
				aWeapon->SetStatic(false);
				cWorldManager::AddObject( aWeapon, true );
				break;
			}
			
			//Food
			case 2:
			{
				cFood* aFood = new cFood(cWorldManager::NewGUID_Object(),locItem,dwModelNumber,pcModel->m_wIcon,Name,Description, pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_dwVitalID, pcModel->m_vital_affect);
				aFood->SetStatic(false);
				cWorldManager::AddObject( aFood, true );
				break;
			}
			
			//Armor
			case 3:
			{
				cArmor* aArmor = new cArmor(cWorldManager::NewGUID_Object(),locItem,dwModelNumber,1.0,TRUE,pcModel->m_wIcon,Name,Description);
				aArmor->SetStatic(false);
				cWorldManager::AddObject(aArmor,true);
				break;
			}
			//Book
			case 4:
			{
				cBooks* aBook = new cBooks(cWorldManager::NewGUID_Object(),locItem,dwModelNumber,pcModel->m_wIcon,Name,Description);
				aBook->SetStatic(false);
				cWorldManager::AddObject(aBook,true);
				break;
			}
			//Scrolls
			case 5:
			{
				cScrolls* aScroll = new cScrolls(cWorldManager::NewGUID_Object(),locItem,dwModelNumber,1.0,TRUE,pcModel->m_wIcon,Name,Description);
				aScroll->SetStatic(false);
				cWorldManager::AddObject(aScroll,true);
				break;
			}
			//Healing Kits
			case 6:
			{
				cHealingKits* aHealingKit = new cHealingKits(cWorldManager::NewGUID_Object(),locItem,dwModelNumber,1.0,TRUE,pcModel->m_wIcon,Name,Description,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_wUses,pcModel->m_wUseLimit);
				aHealingKit->SetStatic(false);
				cWorldManager::AddObject(aHealingKit,true);
				break;
			}
			//Wands
			case 8:
			{
				cWands* aWand = new cWands(cWorldManager::NewGUID_Object(),locItem,dwModelNumber,1.0,TRUE,pcModel->m_wIcon,Name,Description);
				aWand->SetStatic(false);
				cWorldManager::AddObject(aWand,true);
				break;
			}
			//Pyreals
			/*
			case 9:
			{
				cPyreals* aPyreal = new cPyreals(cWorldManager::NewGUID_Object(),locItem,dwModelNumber,1.0,TRUE,pcModel->m_wIcon,Name,Description, pcModel->m_wStack, pcModel->m_wStackLimit);
				aPyreal->SetStatic(false);
				cWorldManager::AddObject(aPyreal,true);
				break;
			}
*/
//		UpdateConsole( "+" );
		}
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Loads world objects from the database.
 *
 *	Author: Cubem0j0
 */
void cMasterServer::LoadWorldObjects( )
{
	char		szCommand[512];
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	RETCODE		retcode;
	WORD		wWOcount = 0;
	WORD		wModel;
	WORD		wIcon;
	char		szWorldObjects[150];
	char		WOName[50];
	char		WODescription[20];
	WORD		wType;
	int			iObjectType = 0;

	sprintf( szCommand, "SELECT COUNT(ID) FROM worldobjects;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wWOcount, sizeof( wWOcount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		wWOcount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szWorldObjects, " Loading %d WorldObjects ... ",wWOcount );
	UpdateConsole((char *)szWorldObjects);

	cLocation locWO;

	sprintf( szCommand, "SELECT * FROM worldobjects;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 3;

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wType, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flX, sizeof( &locWO.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flY, sizeof( &locWO.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flZ, sizeof( &locWO.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flA, sizeof( &locWO.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flB, sizeof( &locWO.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flC, sizeof( &locWO.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locWO.m_flW, sizeof( &locWO.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, WOName, sizeof( WOName ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, WODescription, sizeof( WODescription ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( WORD ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &iObjectType, sizeof( INT ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		
	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
		sscanf(dwLandblock,"%08x",&locWO.m_dwLandBlock);		
		sscanf(dwPosX,"%08x",&locWO.m_flX);
		sscanf(dwPosY,"%08x",&locWO.m_flY);
		sscanf(dwPosZ,"%08x",&locWO.m_flZ);
		sscanf(dwOrientW,"%08x",&locWO.m_flA);
		sscanf(dwOrientX,"%08x",&locWO.m_flB);
		sscanf(dwOrientY,"%08x",&locWO.m_flC);
		sscanf(dwOrientZ,"%08x",&locWO.m_flW);

		cWorldObject* aObject = new cWorldObject(wType, cWorldManager::NewGUID_Object(), &locWO, WOName, WODescription);
		aObject->SetData(wModel,wIcon);
		aObject->SetObjectType(iObjectType);
		cWorldManager::AddObject( aObject, TRUE );

//		UpdateConsole( "+" );
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Loads housing objects from database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadHousing( )
{
	char		szCommand[512];
	RETCODE		retcode;
	WORD		wHouseCount = 0;
	char		szHouse[50];

	char		HouseIDBuff[9];
	DWORD		HouseID;
	char		OwnerIDBuff[9];
	DWORD		OwnerID;
	WORD		wHouseType;
	cLocation	locHouse;
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	char		HouseName[100];
	char		HouseDescription[500];
	WORD		wModel;
	WORD		wIcon;
	DWORD		IsOpen;

	sprintf( szCommand, "SELECT COUNT(ID) FROM houses;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wHouseCount, sizeof( wHouseCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		wHouseCount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szHouse, " Loading %d house objects ... ",wHouseCount );
	UpdateConsole((char *)szHouse);

	for ( int iTmp = 1; iTmp < wHouseCount + 1; iTmp++)
	{
		sprintf( szCommand, "SELECT * FROM houses WHERE ID=%d;", iTmp );
	
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		
		DWORD ID;
		int iCol = 1;

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &ID, sizeof( DWORD ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, HouseIDBuff, sizeof( HouseIDBuff ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wHouseType, sizeof( wHouseType ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHouse.m_flX, sizeof( &locHouse.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHouse.m_flY, sizeof( &locHouse.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHouse.m_flZ, sizeof( &locHouse.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHouse.m_flA, sizeof( &locHouse.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHouse.m_flB, sizeof( &locHouse.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHouse.m_flC, sizeof( &locHouse.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHouse.m_flW, sizeof( &locHouse.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, HouseName, sizeof( HouseName ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, HouseDescription, sizeof( HouseDescription ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &IsOpen, sizeof( IsOpen ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
		if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
		{
			// to access houses, house GUIDs must be the same as those encoded into the cell.dat
			sscanf(HouseIDBuff,"%08x",&HouseID);
			sscanf(OwnerIDBuff,"%08x",&OwnerID);
			sscanf(dwLandblock,"%08x",&locHouse.m_dwLandBlock);
			sscanf(dwPosX,"%08x",&locHouse.m_flX);
			sscanf(dwPosY,"%08x",&locHouse.m_flY);
			sscanf(dwPosZ,"%08x",&locHouse.m_flZ);
			sscanf(dwOrientW,"%08x",&locHouse.m_flA);
			sscanf(dwOrientX,"%08x",&locHouse.m_flB);
			sscanf(dwOrientY,"%08x",&locHouse.m_flC);
			sscanf(dwOrientZ,"%08x",&locHouse.m_flW);

//			cHouse* aHouse = new cHouse(HouseName,HouseDescription,wHouseType,cWorldManager::NewGUID_Object(),&locHouse);
			cHouse* aHouse = new cHouse(HouseName,HouseDescription,wHouseType,HouseID,&locHouse);
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			aHouse->SetData(wModel,wIcon);
			aHouse->SetIsOpen(IsOpen);
			aHouse->SetOwner(OwnerID);
			cWorldManager::AddObject( aHouse, TRUE );

//			UpdateConsole( "+" );
		} else {
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		}
	}
}

/**
 *	Loads covenant crystals from the database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadCovenants( )
{
	char		szCommand[512];
	RETCODE		retcode;
	char		szCovenant[50];
	DWORD		dwCovenantMax;
	DWORD		dwCovenantCount;

	char		CovenantIDBuff[9];
	DWORD		CovenantID;
	char		OwnerIDBuff[9];
	DWORD		OwnerID;
	DWORD		dwHouseID;
	WORD		wHouseType;
	WORD		dwCrystalType;
	cLocation	locCovenant;
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	char		CrystalName[100];
	char		CrystalDescription[500];
	WORD		wAnimconfig;
	WORD		wModel;
	WORD		wIcon;
	WORD		wIsPaid;

	sprintf( szCommand, "SELECT MAX(ID), COUNT(ID) FROM houses_covenants;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );											CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwCovenantMax, sizeof( dwCovenantMax ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwCovenantCount, sizeof( dwCovenantCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwCovenantMax = 0;
		dwCovenantCount = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szCovenant, " Loading %d covenant crystals ... ",dwCovenantCount );
	UpdateConsole((char *)szCovenant);

	for ( int iTmp = 1; iTmp < dwCovenantMax + 1; iTmp++)
	{
		sprintf( szCommand, "SELECT * FROM houses_covenants WHERE ID=%d;", iTmp ); 
	
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
	
		int iCol = 2;	
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, CovenantIDBuff, sizeof( CovenantIDBuff ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwHouseID, sizeof( dwHouseID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wHouseType, sizeof( wHouseType ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &dwCrystalType, sizeof( dwCrystalType ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flX, sizeof( &locCovenant.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flY, sizeof( &locCovenant.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flZ, sizeof( &locCovenant.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flA, sizeof( &locCovenant.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flB, sizeof( &locCovenant.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flC, sizeof( &locCovenant.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flW, sizeof( &locCovenant.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, CrystalName, sizeof( CrystalName ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, CrystalDescription, sizeof( CrystalDescription ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wAnimconfig, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( WORD ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIsPaid, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
		{
			sscanf(CovenantIDBuff,"%08x",&CovenantID);
			sscanf(OwnerIDBuff,"%08x",&OwnerID);
			sscanf(dwLandblock,"%08x",&locCovenant.m_dwLandBlock);
			sscanf(dwPosX,"%08x",&locCovenant.m_flX);
			sscanf(dwPosY,"%08x",&locCovenant.m_flY);
			sscanf(dwPosZ,"%08x",&locCovenant.m_flZ);
			sscanf(dwOrientW,"%08x",&locCovenant.m_flA);
			sscanf(dwOrientX,"%08x",&locCovenant.m_flB);
			sscanf(dwOrientY,"%08x",&locCovenant.m_flC);
			sscanf(dwOrientZ,"%08x",&locCovenant.m_flW);

			cCovenant* aCovenant = new cCovenant(dwCrystalType,CovenantID,dwHouseID,CrystalName,CrystalDescription,&locCovenant);

			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			aCovenant->SetData(wAnimconfig,wModel,wIcon);
			aCovenant->SetOwner(OwnerID);
			cWorldManager::AddObject( aCovenant, TRUE );

//			UpdateConsole( "+" );
		} else {
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		}
	}
}

/**
 *	Loads housing hooks from the database.
 */
void cMasterServer::LoadHooks( )
{
	char		szCommand[512];
	RETCODE		retcode;
	DWORD		dwHookMax;
	DWORD		dwHookCount;
	char		szHook[50];

	char		HookIDBuff[9];
	DWORD		HookID;
	char		OwnerIDBuff[9];
	DWORD		OwnerID;
	DWORD		dwHouseID;
	WORD		dwHookType;
	cLocation	locHook;
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	char		HookName[100];
	char		HookDescription[500];
	WORD		wAnimconfig;
	WORD		wSoundSet;
	WORD		wModel;
	WORD		wIcon;
	
	sprintf( szCommand, "SELECT MAX(ID), COUNT(ID) FROM houses_hooks;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwHookMax, sizeof( dwHookMax ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwHookCount, sizeof( dwHookCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
		
	if( retcode == SQL_NO_DATA )
	{
		dwHookMax = 0;
		dwHookCount = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szHook, " Loading %d housing hooks ... ",dwHookCount );
	UpdateConsole((char *)szHook);

	for ( int iTmp = 1; iTmp < dwHookMax + 1; iTmp++)
	{
		sprintf( szCommand, "SELECT * FROM houses_hooks WHERE ID=%d;", iTmp );
	
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		
		int iCol = 2;

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, HookIDBuff, sizeof( HookIDBuff ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwHouseID, sizeof( dwHouseID ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &dwHookType, sizeof( dwHookType ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHook.m_flX, sizeof( &locHook.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHook.m_flY, sizeof( &locHook.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHook.m_flZ, sizeof( &locHook.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHook.m_flA, sizeof( &locHook.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHook.m_flB, sizeof( &locHook.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHook.m_flC, sizeof( &locHook.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locHook.m_flW, sizeof( &locHook.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, HookName, sizeof( HookName ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, HookDescription, sizeof( HookDescription ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wAnimconfig, sizeof( WORD ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSoundSet, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
		if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
		{
			sscanf(HookIDBuff,"%08x",&HookID);
			sscanf(OwnerIDBuff,"%08x",&OwnerID);
			sscanf(dwLandblock,"%08x",&locHook.m_dwLandBlock);
			sscanf(dwPosX,"%08x",&locHook.m_flX);
			sscanf(dwPosY,"%08x",&locHook.m_flY);
			sscanf(dwPosZ,"%08x",&locHook.m_flZ);
			sscanf(dwOrientW,"%08x",&locHook.m_flA);
			sscanf(dwOrientX,"%08x",&locHook.m_flB);
			sscanf(dwOrientY,"%08x",&locHook.m_flC);
			sscanf(dwOrientZ,"%08x",&locHook.m_flW);

			cHooks* aHook = new cHooks(dwHookType,HookID,dwHouseID,HookName,HookDescription,&locHook);
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			cWorldManager::AddObject( aHook, TRUE );
			aHook->SetData(wAnimconfig,wSoundSet,wModel,wIcon);

//			UpdateConsole( "+" );
		} else {
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		}
	}
}

/**
 *	Loads housing storage chests from the database.
 */
void cMasterServer::LoadStorage( )
{
	char		szCommand[512];
	RETCODE		retcode;
	DWORD		dwStorageMax;
	DWORD		dwStorageCount;
	char		szStorage[50];

	char		StorageName[100];
	char		StorageDescription[500];
	char		StorageIDBuff[9];
	DWORD		StorageID;
	char		OwnerIDBuff[9];
	DWORD		OwnerID;
	DWORD		dwHouseID;
	WORD		dwStorageType;
	
	cLocation	locStorage;
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];

	sprintf( szCommand, "SELECT MAX(ID), COUNT(ID) FROM houses_storage;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwStorageMax, sizeof( dwStorageMax ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwStorageCount, sizeof( dwStorageCount ), NULL );CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwStorageMax = 0;
		dwStorageCount = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szStorage, " Loading %d housing storage chests ... ",dwStorageCount );
	UpdateConsole((char *)szStorage);

	for ( int iTmp = 1; iTmp < dwStorageMax + 1; iTmp++)
	{
		sprintf( szCommand, "SELECT * FROM houses_storage WHERE ID=%d;", iTmp );
	
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );										CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		
		int iCol = 2;
		
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, StorageIDBuff, sizeof( StorageIDBuff ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwHouseID, sizeof( dwHouseID ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &dwStorageType, sizeof( dwStorageType ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locStorage.m_flX, sizeof( &locStorage.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locStorage.m_flY, sizeof( &locStorage.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locStorage.m_flZ, sizeof( &locStorage.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locStorage.m_flA, sizeof( &locStorage.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locStorage.m_flB, sizeof( &locStorage.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locStorage.m_flC, sizeof( &locStorage.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locStorage.m_flW, sizeof( &locStorage.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, StorageName, sizeof( StorageName ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, StorageDescription, sizeof( StorageDescription ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
		{
			sscanf(StorageIDBuff,"%08x",&StorageID);
			sscanf(OwnerIDBuff,"%08x",&OwnerID);
			sscanf(dwLandblock,"%08x",&locStorage.m_dwLandBlock);
			sscanf(dwPosX,"%08x",&locStorage.m_flX);
			sscanf(dwPosY,"%08x",&locStorage.m_flY);
			sscanf(dwPosZ,"%08x",&locStorage.m_flZ);
			sscanf(dwOrientW,"%08x",&locStorage.m_flA);
			sscanf(dwOrientX,"%08x",&locStorage.m_flB);
			sscanf(dwOrientY,"%08x",&locStorage.m_flC);
			sscanf(dwOrientZ,"%08x",&locStorage.m_flW);

			cStorage* aStorage = new cStorage(dwStorageType,StorageID,dwHouseID,StorageName,StorageDescription,&locStorage);
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			cWorldManager::AddObject( aStorage, TRUE );

//			UpdateConsole( "+" );
		} else {
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		}
	}
}

/**
 *	Loads doors from the database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadDoors( )
{
	char			szCommand[512];
	RETCODE			retcode;
	DWORD			dwDoorMax;
	DWORD			dwDoorCount;
	char			dwLandblock[9];
	char			dwPosX[9];
	char			dwPosY[9];
	char			dwPosZ[9];
	char			dwOrientW[9];
	char			dwOrientX[9];
	char			dwOrientY[9];
	char			dwOrientZ[9];
	WORD			wAnimconfig;
	WORD			wSoundSet;
	WORD			wModel;
	WORD			wIcon;
	char			szDoors[50];
	char			DoorName[100];
	char			DoorDescription[500];
	WORD			wType;

	sprintf( szCommand, "SELECT MAX(ID), COUNT(ID) FROM gameobjects_doors;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwDoorMax, sizeof( dwDoorMax ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwDoorCount, sizeof( dwDoorCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwDoorCount = 0;
		dwDoorMax = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szDoors, " Loading %d doors ... ",dwDoorCount );
	UpdateConsole((char *)szDoors);

	cLocation locDoor;

	sprintf( szCommand, "SELECT * FROM gameobjects_doors;" );
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );												CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	int iCol =	3;

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wType, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, DoorName, sizeof( DoorName ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, DoorDescription, sizeof( DoorDescription ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wAnimconfig, sizeof( WORD ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSoundSet, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		sscanf(dwLandblock,"%08x",&locDoor.m_dwLandBlock);
		sscanf(dwPosX,"%08x",&locDoor.m_flX);
		sscanf(dwPosY,"%08x",&locDoor.m_flY);
		sscanf(dwPosZ,"%08x",&locDoor.m_flZ);
		sscanf(dwOrientW,"%08x",&locDoor.m_flA);
		sscanf(dwOrientX,"%08x",&locDoor.m_flB);
		sscanf(dwOrientY,"%08x",&locDoor.m_flC);
		sscanf(dwOrientZ,"%08x",&locDoor.m_flW);

		cDoor* aDoor = new cDoor(wType, cWorldManager::NewGUID_Object(), &locDoor, DoorName, DoorDescription);
		aDoor->SetData(wAnimconfig,wSoundSet,wModel,wIcon);
		cWorldManager::AddObject( aDoor, TRUE );
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	
}

/**
 *	Loads chests from the database.
 *
 *	Author: Cubem0j0
 */
void cMasterServer::LoadChests( )
{
	char			szCommand[512];
	char			dwLandblock[9];
	char			dwPosX[9];
	char			dwPosY[9];
	char			dwPosZ[9];
	char			dwOrientW[9];
	char			dwOrientX[9];
	char			dwOrientY[9];
	char			dwOrientZ[9];
	RETCODE			retcode;
	WORD			wChestcount = 0;
	WORD			wAnimconfig;
	WORD			wSoundSet;
	WORD			wModel;
	WORD			wIcon;
	DWORD			dwObject1;
	DWORD			dwObject2;
	char			szChests[50];
	char			ChestName[100];
	char			ChestDescription[500];
	char			strdwObject1[9];
	char			strdwObject2[9];
	WORD			wType;

	int				IsLocked;
	int				LockDiff;

	sprintf( szCommand, "SELECT COUNT(ID) FROM gameobjects_chests;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wChestcount, sizeof( wChestcount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		wChestcount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szChests, " Loading %d chests ... ",wChestcount );
	UpdateConsole((char *)szChests);

	cLocation locChest;

	sprintf( szCommand, "SELECT * FROM gameobjects_chests;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 3;

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wType, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locChest.m_flX, sizeof( &locChest.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locChest.m_flY, sizeof( &locChest.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locChest.m_flZ, sizeof( &locChest.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locChest.m_flA, sizeof( &locChest.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locChest.m_flB, sizeof( &locChest.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locChest.m_flC, sizeof( &locChest.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locChest.m_flW, sizeof( &locChest.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ChestName, sizeof( ChestName ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ChestDescription, sizeof( ChestDescription ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wAnimconfig, sizeof( WORD ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSoundSet, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( WORD ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, strdwObject1, sizeof( strdwObject1 ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, strdwObject2, sizeof( strdwObject2 ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &IsLocked, sizeof( INT ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &LockDiff, sizeof( INT ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
		sscanf(dwLandblock,"%08x",&locChest.m_dwLandBlock);
		sscanf(dwPosX,"%08x",&locChest.m_flX);
		sscanf(dwPosY,"%08x",&locChest.m_flY);
		sscanf(dwPosZ,"%08x",&locChest.m_flZ);
		sscanf(dwOrientW,"%08x",&locChest.m_flA);
		sscanf(dwOrientX,"%08x",&locChest.m_flB);
		sscanf(dwOrientY,"%08x",&locChest.m_flC);
		sscanf(dwOrientZ,"%08x",&locChest.m_flW);

		sscanf(strdwObject1,"%08x",&dwObject1);
		sscanf(strdwObject2,"%08x",&dwObject2);

		cChest* aChest = new cChest(wType, cWorldManager::NewGUID_Object(), &locChest, ChestName, ChestDescription);
		aChest->SetData(wAnimconfig,wSoundSet,wModel,wIcon,dwObject1,dwObject2);
		aChest->SetIsLocked(IsLocked);
		aChest->SetLDiff(LockDiff);
		cWorldManager::AddObject( aChest, TRUE );

//		UpdateConsole( "+" );
	}
	//retcode = SQLFetch( cDatabase::m_hStmt );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Loads lifestones from the database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadLifestones( )
{
	char			szCommand[512];
	char			dwLandblock[9];
	char			dwPosX[9];
	char			dwPosY[9];
	char			dwPosZ[9];
	char			dwOrientW[9];
	char			dwOrientX[9];
	char			dwOrientY[9];
	char			dwOrientZ[9];
	RETCODE			retcode;
	WORD			wLScount = 0;
	WORD			wAnimconfig;
	WORD			wSoundSet;
	WORD			wModel;
	WORD			wIcon;
	DWORD			dwObject1;
	DWORD			dwObject2;
	char			szLifestones[50];
	char			LSName[100];
	char			LSDescription[500];
	char			strdwObject1[9];
	char			strdwObject2[9];
	WORD			wType;

	sprintf( szCommand, "SELECT COUNT(ID) FROM gameobjects_lifestones;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );							CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );														CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wLScount, sizeof( wLScount ), NULL );CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		wLScount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szLifestones, " Loading %d lifestones ... ",wLScount );
	UpdateConsole((char *)szLifestones);

	cLocation locLifestone;

	sprintf( szCommand, "SELECT * FROM gameobjects_lifestones;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 3;

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wType, sizeof( WORD ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locLifestone.m_flX, sizeof( &locLifestone.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locLifestone.m_flY, sizeof( &locLifestone.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locLifestone.m_flZ, sizeof( &locLifestone.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locLifestone.m_flA, sizeof( &locLifestone.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locLifestone.m_flB, sizeof( &locLifestone.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locLifestone.m_flC, sizeof( &locLifestone.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locLifestone.m_flW, sizeof( &locLifestone.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, LSName, sizeof( LSName ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, LSDescription, sizeof( LSDescription ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wAnimconfig, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSoundSet, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( WORD ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( WORD ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, strdwObject1, sizeof( strdwObject1 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, strdwObject2, sizeof( strdwObject2 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
		sscanf(dwLandblock,"%08x",&locLifestone.m_dwLandBlock);
		sscanf(dwPosX,"%08x",&locLifestone.m_flX);
		sscanf(dwPosY,"%08x",&locLifestone.m_flY);
		sscanf(dwPosZ,"%08x",&locLifestone.m_flZ);
		sscanf(dwOrientW,"%08x",&locLifestone.m_flA);
		sscanf(dwOrientX,"%08x",&locLifestone.m_flB);
		sscanf(dwOrientY,"%08x",&locLifestone.m_flC);
		sscanf(dwOrientZ,"%08x",&locLifestone.m_flW);
		sscanf(strdwObject1,"%08x",&dwObject1);
		sscanf(strdwObject2,"%08x",&dwObject2);

		cLifestone* aLifestone = new cLifestone(wType, cWorldManager::NewGUID_Object(), &locLifestone, LSName, LSDescription);
		aLifestone->SetData(wAnimconfig,wSoundSet,wModel,wIcon,dwObject1,dwObject2);
		cWorldManager::AddObject( aLifestone, TRUE );

//		UpdateConsole( "+" );
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );			CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Loads portals from the database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadPortals( )
{
	char			szCommand[512];
	RETCODE			retcode;
	
	// Load Portals
	cLocation	location;
	cLocation	destination;
	char		PortalColor[25];
	char		PortalName[100];
	char		PortalDescription[500];
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	char		dwDestLandblock[9];
	char		dwDestPosX[9];
	char		dwDestPosY[9];
	char		dwDestPosZ[9];
	char		dwDestOrientW[9];
	char		dwDestOrientX[9];
	char		dwDestOrientY[9];
	char		dwDestOrientZ[9];

	DWORD		dwMinLevel;
	DWORD		dwMaxLevel;

	DWORD		dwPortalMax;
	DWORD		dwPortalCount;
	char		szPortals[50];

	sprintf( szCommand, "SELECT MAX(ID), COUNT(ID) FROM gameobjects_portals;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwPortalMax, sizeof( dwPortalMax ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwPortalCount, sizeof( dwPortalCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwPortalMax = 0;
		dwPortalCount = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
	
	
	sprintf( szPortals, " Loading %d portals ... ",dwPortalCount );
	UpdateConsole((char *)szPortals);

	
		sprintf( szCommand, "SELECT * FROM gameobjects_portals;");


		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)	

		int iCol = 4;
							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, PortalName, sizeof( PortalName ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, PortalDescription, sizeof( PortalDescription ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, PortalColor, sizeof( PortalColor ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwDestLandblock, sizeof( dwDestLandblock ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwDestPosX, sizeof( dwDestPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwDestPosY, sizeof( dwDestPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwDestPosZ, sizeof( dwDestPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwDestOrientW, sizeof( dwDestOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwDestOrientX, sizeof( dwDestOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwDestOrientY, sizeof( dwDestOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwDestOrientZ, sizeof( dwDestOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwMinLevel, sizeof( &dwMinLevel ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwMaxLevel, sizeof( &dwMaxLevel ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		

		for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; i++ ) 
		{

			sscanf(dwLandblock,"%08x",&location.m_dwLandBlock);
			sscanf(dwPosX,"%08x",&location.m_flX);
			sscanf(dwPosY,"%08x",&location.m_flY);
			sscanf(dwPosZ,"%08x",&location.m_flZ);
			sscanf(dwOrientW,"%08x",&location.m_flA);
			sscanf(dwOrientX,"%08x",&location.m_flB);
			sscanf(dwOrientY,"%08x",&location.m_flC);
			sscanf(dwOrientZ,"%08x",&location.m_flW);

			sscanf(dwDestLandblock,"%08x",&destination.m_dwLandBlock);
			sscanf(dwDestPosX,"%08x",&destination.m_flX);
			sscanf(dwDestPosY,"%08x",&destination.m_flY);
			sscanf(dwDestPosZ,"%08x",&destination.m_flZ);
			sscanf(dwDestOrientW,"%08x",&destination.m_flA);
			sscanf(dwDestOrientX,"%08x",&destination.m_flB);
			sscanf(dwDestOrientY,"%08x",&destination.m_flC);
			sscanf(dwDestOrientZ,"%08x",&destination.m_flW);

			DWORD color;
			if(lstrcmpi(PortalColor,"blue")==0)
				color = ColorBlue;
			else if(lstrcmpi(PortalColor,"green")==0)
				color = ColorGreen;
			else if(lstrcmpi(PortalColor,"orange")==0)
				color = ColorBrown;
			else if(lstrcmpi(PortalColor,"red")==0)
				color = ColorRed;
			else if(lstrcmpi(PortalColor,"yellow")==0)
				color = ColorYellow;
			else if(lstrcmpi(PortalColor,"purple")==0)
				color = ColorMagenta;
			else if(lstrcmpi(PortalColor,"white")==0)
				color = ColorWhite;
			else 
				color = ColorMagenta;

			cPortal* cpl = new cPortal( cWorldManager::NewGUID_Object(),color,&location, &destination, PortalName,PortalDescription, dwMinLevel, dwMaxLevel );
			cWorldManager::AddObject( cpl );
		} 
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
}

/**
 *	Loads altars from the database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadAltars( )
{
	char		szCommand[512];
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	RETCODE		retcode;
	int i;
	WORD		wAltarcount = 0;
	char		szAltars[50];

	sprintf( szCommand, "SELECT COUNT(ID) FROM gameobjects_altars;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );									CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wAltarcount, sizeof( wAltarcount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		wAltarcount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szAltars, " Loading %d altars ... ",wAltarcount );
	UpdateConsole((char *)szAltars);

	// PK ALtar Load
	cLocation PKAltar;

	sprintf( szCommand, "SELECT * FROM gameobjects_altars WHERE Type = 'PKALTAR';" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 4;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flX, sizeof( &NPKAltar.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flY, sizeof( &NPKAltar.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flZ, sizeof( &NPKAltar.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flA, sizeof( &NPKAltar.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flB, sizeof( &NPKAltar.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flC, sizeof( &NPKAltar.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flW, sizeof( &NPKAltar.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				
	for (i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ){
		sscanf(dwLandblock,"%08x",&PKAltar.m_dwLandBlock);
		sscanf(dwPosX,"%08x",&PKAltar.m_flX);
		sscanf(dwPosY,"%08x",&PKAltar.m_flY);
		sscanf(dwPosZ,"%08x",&PKAltar.m_flZ);
		sscanf(dwOrientW,"%08x",&PKAltar.m_flA);
		sscanf(dwOrientX,"%08x",&PKAltar.m_flB);
		sscanf(dwOrientY,"%08x",&PKAltar.m_flC);
		sscanf(dwOrientZ,"%08x",&PKAltar.m_flW);

		cAltar* anAltar = new cAltar(1,cWorldManager::NewGUID_Object(),&PKAltar,"Altar of Bael'Zharon","Double click to go PK");
		cWorldManager::AddObject( anAltar, TRUE );

//		UpdateConsole( "+" );
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );			CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	// NPK Altar Load
	cLocation NPKAltar;

	sprintf( szCommand, "SELECT * FROM gameobjects_altars WHERE Type = 'NPKALTAR';" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );

	iCol = 4;											   
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flX, sizeof( &NPKAltar.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flY, sizeof( &NPKAltar.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flZ, sizeof( &NPKAltar.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flA, sizeof( &NPKAltar.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flB, sizeof( &NPKAltar.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flC, sizeof( &NPKAltar.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &NPKAltar.m_flW, sizeof( &NPKAltar.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for (i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		sscanf(dwLandblock,"%08x",&NPKAltar.m_dwLandBlock);
		sscanf(dwPosX,"%08x",&PKAltar.m_flX);
		sscanf(dwPosY,"%08x",&PKAltar.m_flY);
		sscanf(dwPosZ,"%08x",&PKAltar.m_flZ);
		sscanf(dwOrientW,"%08x",&PKAltar.m_flA);
		sscanf(dwOrientX,"%08x",&PKAltar.m_flB);
		sscanf(dwOrientY,"%08x",&PKAltar.m_flC);
		sscanf(dwOrientZ,"%08x",&PKAltar.m_flW);
		
		//UpdateConsole( "+" );

		cAltar* poopo = new cAltar(0,cWorldManager::NewGUID_Object(),&NPKAltar,"Altar of Asheron","Double Click to go NPK");
		cWorldManager::AddObject( poopo, TRUE );
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

}

/**
 *	Loads pre-defined non-player character (NPC) models from the database.
 *
 *	Note:  Used to speed up the loading of NPCs.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadNPCModels( )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			szMessage[155];
	DWORD			dwModelCount;
	DWORD			dwNumModels;
	char			ModelName[75];
	DWORD			dwModelID;
	BYTE			bPaletteChange;
	sPaletteChange	vPalettes[255];
	BYTE			bTextureChange;
	sTextureChange	vTextures[255];
	BYTE			bModelChange;
	sModelChange	vModels[255];
	char			szModel[5];
	char			szIcon[5];
//	WORD			wModel;
//	WORD			wIcon;
	char			szPaletteCode[5];
	char			szModelNumber[5];
	//char			bInitialAnimation[200];
	float			flScale;

	// Start the loading

	sprintf( szCommand, "SELECT MAX(ModelNum),COUNT(ModelNum) FROM npcs_models;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwModelCount, sizeof( dwModelCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwNumModels, sizeof( dwNumModels ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwModelCount = 0;
		dwNumModels = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMessage, " Loading %d pre-defined NPC models ... ",dwNumModels );
	UpdateConsole((char *)szMessage);

	// Now all that data needs to be loaded
	sprintf( szCommand, "SELECT * FROM npcs_models ORDER BY ID;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	int iCol = 2;
	
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelID, sizeof( dwModelID ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelName, sizeof( ModelName ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flScale, sizeof( flScale ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	//TODO: Edit the Modeldata table..remove a bunch of stuff.
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bPaletteChange, sizeof( BYTE ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bTextureChange, sizeof( BYTE ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bModelChange , sizeof( BYTE ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szModel, sizeof( szModel ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szIcon, sizeof( szIcon ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szPaletteCode, sizeof( szPaletteCode ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szModelNumber, sizeof( szModelNumber ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		cNPCModels* pcModel = new cNPCModels( dwModelID );


			pcModel->m_strName.assign(ModelName);
			pcModel->m_flScale			= flScale;
			pcModel->m_bPaletteChange	= bPaletteChange;
			pcModel->m_wPaletteVector	= dwModelID;
			pcModel->m_bTextureChange	= bTextureChange;
			pcModel->m_wTextureVector	= dwModelID;
			pcModel->m_bModelChange		= bModelChange;
			pcModel->m_wModelVector		= dwModelID;

			sscanf(szModel,"%04x",&pcModel->m_wModel);
			sscanf(szIcon,"%04x",&pcModel->m_wIcon);
			sscanf(szPaletteCode,"%04x",&pcModel->m_wPaletteCode);
			sscanf(szModelNumber,"%08x",&pcModel->m_dwModelNumber);
/*
		int b = 0;
		char	cTemp[2];
		for(int i = 0; i < (dwUnknownCount); i++)
		{
			CopyMemory( &cTemp[0], &bInitialAnimation[b], 2 );
			sscanf(cTemp,"%08x",&pcModel->m_bInitialAnimation[i]);
			b = b + 2;
		}
*/
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	//===============================================================
	// Load all the Vector Data for Each Model ID
	// 
	//===============================================================
	for ( DWORD d = 0; d < dwModelCount+1; d++)
	{
		cNPCModels *pcModel = cNPCModels::Hash_Find( d );
		if(pcModel) 
		{
			sprintf( szCommand, "SELECT * FROM npcs_vector_palettes WHERE modelvector=%d ORDER BY VectorCount;", pcModel->m_wPaletteVector);

			// Load Palette vector into array

			// Define Temp Variables
			BYTE    bVectorCount; 
			char	readbuff01[5];
			char	readbuff02[3];
			char	readbuff03[3];

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( bVectorCount ), NULL );	
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff01 , sizeof( readbuff01 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff02 , sizeof( readbuff02 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff03 , sizeof( readbuff03 ), NULL );		

			for ( int intPcount = 0 ; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS && intPcount < pcModel->m_bPaletteChange +1; ++intPcount )
			{
				// Let's scan that text into Hex
				sscanf(readbuff01,"%04x",&pcModel->m_vectorPal[intPcount].m_wNewPalette);
				sscanf(readbuff02,"%02x",&pcModel->m_vectorPal[intPcount].m_ucOffset);
				sscanf(readbuff03,"%02x",&pcModel->m_vectorPal[intPcount].m_ucLength);
			}
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			// Load Texture Vector into Array

			// Define Temp Variables
			char	readbuff04[3];
			char	readbuff05[5];
			char	readbuff06[5];

			sprintf( szCommand, "SELECT * FROM npcs_vector_textures WHERE modelvector=%d ORDER BY VectorCount;",pcModel->m_wTextureVector);
		
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff04, sizeof( readbuff04 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff05, sizeof( readbuff05 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff06 , sizeof( readbuff06 ), NULL );	

			for ( int intT = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS && intT < pcModel->m_bTextureChange + 1; ++intT )
			{
				// Let's scan that text into Hex
				sscanf(readbuff04,"%02x",&pcModel->m_vectorTex[intT].m_bModelIndex);
				sscanf(readbuff05,"%04x",&pcModel->m_vectorTex[intT].m_wOldTexture);
				sscanf(readbuff06,"%04x",&pcModel->m_vectorTex[intT].m_wNewTexture);
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt ); 
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			//  Load Model Vectors into Array

			// Define Temp Variables
			char	readbuff07[3];
			char	readbuff08[5];

			sprintf( szCommand, "SELECT * FROM npcs_vector_models WHERE modelvector=%d ORDER BY VectorCount;", pcModel->m_wModelVector);

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff07, sizeof( readbuff07 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff08 , sizeof( readbuff08 ), NULL );	

			for ( int intM = 0; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) && (intM < pcModel->m_bModelChange + 1); ++intM )
			{
				// Scan the text to HEX
				sscanf(readbuff07,"%02x",&pcModel->m_vectorMod[intM].m_bModelIndex);
				sscanf(readbuff08,"%04x",&pcModel->m_vectorMod[intM].m_wNewModel);
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt ); 
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		
//			UpdateConsole( "+" );
		}
	}
}

/**
 *	Loads non-player characters (NPCs) from the database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadNPCs( )
{
	char		szCommand[512];
	RETCODE		retcode;
	char		AvatarName[100];
	char		ReadMode[15];
	char		cGender[2];
	WORD		wGender;
	cLocation	avatarloc;
	DWORD		dwSellCategories;
	DWORD		dwMode;
	int			iNumMessages = 0;
	char		Message_1[500];
	char		Message_2[500];
	char		Message_3[500];   
	char		Message_4[500];
	char		Message_5[500];
	char		Message_6[500];
	char		Message_7[500];
	char		Message_8[500];
	char		Message_9[500];
	char		Message_10[500];
	char		NPCLoc[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	WORD		wNPCModel = 0;
	WORD		wNPCcount = 0;
	int			IsVendor = 0;
	char		szNPCs[50];
	char		SellCat[9];
	DWORD		NPC_ID;
	DWORD		qitem_id1;
	DWORD		vendor_item_modelnumber;

	sprintf( szCommand, "SELECT max(ID) FROM npcs;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );															CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wNPCcount, sizeof( wNPCcount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		wNPCcount = 0;

	sprintf( szNPCs, " Loading %d NPCs ... ",wNPCcount );
	UpdateConsole((char *)szNPCs);

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
	
	// Now process all NPC models
	for ( int iTmp = 0; iTmp < wNPCcount + 1; iTmp++)
	{
		sprintf( szCommand, "SELECT * FROM npcs WHERE ID = %d;", iTmp ); 
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );

		int iCol = 3;
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &NPC_ID, sizeof( NPC_ID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, AvatarName, sizeof( AvatarName ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, cGender, sizeof( cGender ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wNPCModel, sizeof( WORD ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ReadMode, sizeof( ReadMode ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &iNumMessages, sizeof( INT ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_1, sizeof( Message_1 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_2, sizeof( Message_2 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_3, sizeof( Message_3 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_4, sizeof( Message_4 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_5, sizeof( Message_5 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_6, sizeof( Message_6 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_7, sizeof( Message_7 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_8, sizeof( Message_8 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_9, sizeof( Message_9 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Message_10, sizeof( Message_10 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
		iCol = 19;
		// Location
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, NPCLoc, sizeof( NPCLoc ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
/*		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &avatarloc.m_flX, sizeof( &avatarloc.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &avatarloc.m_flY, sizeof( &avatarloc.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &avatarloc.m_flZ, sizeof( &avatarloc.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &avatarloc.m_flA, sizeof( &avatarloc.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &avatarloc.m_flB, sizeof( &avatarloc.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &avatarloc.m_flC, sizeof( &avatarloc.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &avatarloc.m_flW, sizeof( &avatarloc.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &IsVendor, sizeof( INT ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR,  SellCat, sizeof( SellCat ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &qitem_id1, sizeof( qitem_id1 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		if( SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS )
		{		
			sscanf(NPCLoc,"%08x",&avatarloc.m_dwLandBlock);
			sscanf(dwPosX,"%08x",&avatarloc.m_flX);
			sscanf(dwPosY,"%08x",&avatarloc.m_flY);
			sscanf(dwPosZ,"%08x",&avatarloc.m_flZ);
			sscanf(dwOrientW,"%08x",&avatarloc.m_flA);
			sscanf(dwOrientX,"%08x",&avatarloc.m_flB);
			sscanf(dwOrientY,"%08x",&avatarloc.m_flC);
			sscanf(dwOrientZ,"%08x",&avatarloc.m_flW);
			sscanf(SellCat, "%08x",&dwSellCategories);

	   		// Set Gender
			tolower(cGender[1]);
			if(cGender[1] == 'f')
			{
				wGender = 0;
			} else {
				wGender = 1;
			}

			cNPC* npc = new cNPC(cWorldManager::NewGUID_Object(), wNPCModel, AvatarName, wGender, &avatarloc, dwSellCategories);
			// Set Message Type
			if(lstrcmpi(ReadMode,"RANDOM")==0)
				dwMode = NPCMODE_RANDOM;
			else if(lstrcmpi(ReadMode,"MULTI")==0)
				dwMode = NPCMODE_MULTI;
			else if(lstrcmpi(ReadMode,"SINGLE")==0)
				dwMode = NPCMODE_SINGLE;
			npc->SetMode(dwMode);	 // Message Mode
			npc->SetNumMessages(iNumMessages);	 // Number of Messages
			npc->SetIsVendor(IsVendor);
			npc->Set_npc_id(NPC_ID);

			npc->m_qitem_id1 = qitem_id1;

			// Store the Messages
			if (iNumMessages >= 1 )	{
				char* sz = strchr(Message_1,' ');
				//*sz = 0;
				npc->SetString(Message_1,0);
			}
			if (iNumMessages >= 2 )	{
				char* sz = strchr(Message_2,' ');
				//*sz = 0;
				npc->SetString(Message_2,1);
			}
			if (iNumMessages >= 3 )	{
				char* sz = strchr(Message_3,' ');
				//*sz = 0;
				npc->SetString(Message_3,2);
			}
			if (iNumMessages >= 4 )	{
				char* sz = strchr(Message_4,' ');
				//*sz = 0;
				npc->SetString(Message_4,3);
			}
			if (iNumMessages >= 5 )	{
				char* sz = strchr(Message_5,' ');
				//*sz = 0;
				npc->SetString(Message_5,4);
			}
			if (iNumMessages >= 6 )	{
				char* sz = strchr(Message_6,' ');
				//*sz = 0;
				npc->SetString(Message_6,5);
			}
			if (iNumMessages >= 7 )	{
				char* sz = strchr(Message_7,' ');
				//*sz = 0;
				npc->SetString(Message_7,6);
			}
			if (iNumMessages >= 8 )	{
				char* sz = strchr(Message_8,' ');
				//*sz = 0;
				npc->SetString(Message_8,7);
			}
			if (iNumMessages >= 9 )	{
				char* sz = strchr(Message_9,' ');
				//*sz = 0;
				npc->SetString(Message_9,8);
			}
			if (iNumMessages == 10 )	{
				char* sz = strchr(Message_10,' ');
				//*sz = 0;
				npc->SetString(Message_10,9);
			}
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			cWorldManager::AddObject( npc, TRUE );

//			UpdateConsole( "+" );

			if (npc->GetIsVendor() == 1)
			{
				sprintf( szCommand, "SELECT * FROM npcs_vendor_items WHERE vendor_id = %d;", npc->Get_npc_id() ); 
				retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLExecute( cDatabase::m_hStmt );
				
				int iCol = 3;
				retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &vendor_item_modelnumber, sizeof( vendor_item_modelnumber ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				
				for (int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
				{
					//Cubem0j0:  We find the model number so we can get some vars.
					cItemModels *pcModel = cItemModels::FindModel(vendor_item_modelnumber);

					switch(pcModel->m_ItemType)
					{
					case 1:	//Weapons
						{
							cWeapon* weapon = new cWeapon(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->m_dwWeaponDamage,pcModel->m_dwWeaponSpeed,pcModel->m_dwWeaponSkill,pcModel->m_dwDamageType,pcModel->m_dWeaponVariance,pcModel->m_dWeaponModifier,pcModel->m_dWeaponPower,pcModel->m_dWeaponAttack);
							npc->v_guids[i] = weapon->GetGUID();
							npc->AddInventory(weapon);
						}
						break;

					case 2:
						{
							cFood* food = new cFood(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wStack, pcModel->m_wBurden, pcModel->m_dwVitalID, pcModel->m_vital_affect);
							npc->v_guids[i] = food->GetGUID();
							npc->AddInventory(food);
						}
						break;

					case 3:	//Armor
						{
							cArmor* armor = new cArmor(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden,pcModel->m_dwArmor_Level,pcModel->m_fProt_Slashing, pcModel->m_fProt_Piercing, pcModel->m_fProt_Bludgeon, pcModel->m_fProt_Fire, pcModel->m_fProt_Cold, pcModel->m_fProt_Acid, pcModel->m_fProt_Electric);
							npc->v_guids[i] = armor->GetGUID();
							npc->AddInventory(armor);
						}
						break;

					case 4:
						break;

					case 5:
						{
							cScrolls* scrolls = new cScrolls(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
							npc->v_guids[i] = scrolls->GetGUID();
							npc->AddInventory(scrolls);
						}
						break;

					case 6:
						break;

					case 7:
						{
							cLockpicks* lockpick = new cLockpicks(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wUses,pcModel->m_wUseLimit);
							npc->v_guids[i] = lockpick->GetGUID();
							npc->AddInventory(lockpick);
						}
						break;

					case 8:	//Casting Items
						{
							cWands* wands = new cWands(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
							npc->v_guids[i] = wands->GetGUID();
							npc->AddInventory(wands);
						}
						break;

					case 9:
						break;

					case 10:	//Mana Stones
						{
							cManaStones* manastones = new cManaStones(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
							npc->v_guids[i] = manastones->GetGUID();
							npc->AddInventory(manastones);
						}
						break;
					
					case 11:	//Missile Weapons/Ammo
						{
							cAmmo* ammo = new cAmmo(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_wStack,pcModel->m_wStackLimit);
							npc->v_guids[i] = ammo->GetGUID();
							npc->AddInventory(ammo);
						}
						break;
				
					case 12:
						{
							cShield* shield = new cShield(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_dwArmor_Level);
							npc->v_guids[i] = shield->GetGUID();
							npc->AddInventory(shield);
						}
						break;

					case 13:
						{
							cSpellComps* comps = new cSpellComps(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wStack,pcModel->m_wBurden);
							npc->v_guids[i] = comps->GetGUID();
							npc->AddInventory(comps);
						}
						break;

					case 14:
						{
							cGems* gem = new cGems(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
							npc->v_guids[i] = gem->GetGUID();
							npc->AddInventory(gem);
						}
						break;

					case 15:
						{
							cTradeNotes* tradenotes = new cTradeNotes(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
							npc->v_guids[i] = tradenotes->GetGUID();
							npc->AddInventory(tradenotes);
						}
						break;
					
					case 16:
						{
							cTradeSkillMats* mats = new cTradeSkillMats(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
							npc->v_guids[i] = mats->GetGUID();
							npc->AddInventory(mats);
						}
						break;
					case 17:
						{

						}
						break;
					case 18:
						{
							cClothes* rags = new cClothes(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
							npc->v_guids[i] = rags->GetGUID();
							npc->AddInventory(rags);
						}
						break;
					case 19:
						{
							cJewelry* ring = new cJewelry(cWorldManager::NewGUID_Object(),npc->GetGUID(),vendor_item_modelnumber,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden);
							npc->v_guids[i] = ring->GetGUID();
							npc->AddInventory(ring);
						}
						break;
					}

				}
				retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
			}

		}  // End For Loop thru NPC objects	
		else
		{
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		}
	} 
}


/**
 *	Loads pre-defined monster models from the database.
 *
 *	Note: Primarily used for monsters, since there are several copies of
 *		  the same data. One model needs to be loaded for each creature,
 *		  which may then be referenced by the Model Number as needed.
 *
 *	TODO: Load monster models from the portal.dat instead.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadMonsterModels( )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			szMessage[155];
	DWORD			dwModelCount;
	DWORD			dwNumModels;

	char			ModelName[75];
	char			ModelDescription[255];
	DWORD			dwModelID;
	BYTE			bPaletteChange;
	sPaletteChange	vPalettes[255];
	BYTE			bTextureChange;

	sTextureChange	vTextures[255];
	BYTE			bModelChange;

	sModelChange	vModels[255];

	char			szFlags1[9];
	WORD			wPortalMode;
	WORD			wUnknown_1;
	WORD			wModel;
	WORD			wIcon;
	char			szAnimConfig[5];
	char			szSoundSet[5];
	
	DWORD			dwUnknown_Blue;
	char			szModelNumber[5];
	float			flScale;
	DWORD			dwUnknown_LightGrey;
	DWORD			dwTrio1[3];
	DWORD			dwMedGrey;
	DWORD			dwBlueGrey;
	WORD			wSeagreen8;
	
	DWORD			dwUnknownCount;
	char			bInitialAnimation[200];
	DWORD			dwUnknown_v2;
	DWORD			dwUnknown_v6;
	DWORD			dwSpecies;

	// Start the loading

	sprintf( szCommand, "SELECT MAX(dwLinker),COUNT(dwLinker) FROM monsters_templates;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwModelCount, sizeof( dwModelCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwNumModels, sizeof( dwNumModels ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwModelCount = 0;
		dwNumModels = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMessage, " Loading %d pre-defined monster models ... ",dwNumModels );
	UpdateConsole((char *)szMessage);

	// Now all that data needs to be loaded
	sprintf( szCommand, "SELECT * FROM monsters_templates ORDER BY ID;" );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelName, sizeof( ModelName ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, ModelDescription, sizeof( ModelDescription ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bPaletteChange, sizeof( BYTE ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bTextureChange, sizeof( BYTE ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bModelChange, sizeof( BYTE ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// dwFlags1		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szFlags1, sizeof( szFlags1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wPortalMode, sizeof( wPortalMode ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wUnknown_1, sizeof( wUnknown_1 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModel, sizeof( wModel ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wIcon, sizeof( wIcon ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// Animconfig, Soundset
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szAnimConfig, sizeof( szAnimConfig ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szSoundSet, sizeof( szSoundSet ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_Blue, sizeof( dwUnknown_Blue ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	// dwModelNumber		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &szModelNumber, sizeof( szModelNumber ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &flScale, sizeof( flScale ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_LightGrey, sizeof( dwUnknown_LightGrey ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[0], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[1], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &dwTrio1[2], sizeof( dwTrio1 ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMedGrey, sizeof( dwMedGrey ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwBlueGrey, sizeof( dwBlueGrey ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wSeagreen8, sizeof( wSeagreen8 ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_v2, sizeof( dwUnknown_v2 ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknown_v6, sizeof( dwUnknown_v6 ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwUnknownCount, sizeof( dwUnknownCount ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &bInitialAnimation, sizeof( bInitialAnimation ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelID, sizeof( dwModelID ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwSpecies, sizeof( dwSpecies ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		cModels* pcModel = new cModels( dwModelID );

		pcModel->m_strName.assign(ModelName);
		pcModel->m_strDescription.assign(ModelDescription);
		pcModel->m_bPaletteChange	= bPaletteChange;
		pcModel->m_wPaletteVector	= dwModelID;
		pcModel->m_bTextureChange	= bTextureChange;
		pcModel->m_wTextureVector	= dwModelID;
		pcModel->m_bModelChange		= bModelChange;
		pcModel->m_wModelVector		= dwModelID;

		sscanf(szFlags1,"%08x",&pcModel->m_dwFlags1);
		
		pcModel->m_wPortalMode		= wPortalMode;
		pcModel->m_wUnknown_1		= wUnknown_1;
		pcModel->m_wModel			= wModel;
		pcModel->m_wIcon			= wIcon;

		sscanf(szAnimConfig,"%08x",&pcModel->m_wAnimConfig);
		sscanf(szSoundSet,"%08x",&pcModel->m_wSoundSet);
		
		pcModel->m_dwUnknown_Blue	= dwUnknown_Blue;

		sscanf(szModelNumber,"%08x",&pcModel->m_dwModelNumber);

		pcModel->m_flScale			= flScale;
		pcModel->m_dwUnknown_LightGrey = dwUnknown_LightGrey;
		pcModel->m_dwTrio1[0]		= dwTrio1[0];
		pcModel->m_dwTrio1[1]		= dwTrio1[1];
		pcModel->m_dwTrio1[2]		= dwTrio1[2];
		pcModel->m_dwMedGrey		= dwMedGrey;
		pcModel->m_dwBlueGrey		= dwBlueGrey;
		pcModel->m_wSeagreen8		= wSeagreen8;
		pcModel->m_dwUnknown_v2		= dwUnknown_v2;
		pcModel->m_dwUnknown_v6		= dwUnknown_v6;
		pcModel->m_dwUnknownCount	= dwUnknownCount;
		pcModel->m_dwSpecies		= dwSpecies;
		int b = 0;
		char	cTemp[2];

		for(int i = 0; i < (dwUnknownCount); i++) {
			CopyMemory( &cTemp[0], &bInitialAnimation[b], 2 );
			sscanf(cTemp,"%08x",&pcModel->m_bInitialAnimation[i]);
			b = b + 2;
		}
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	//===============================================================
	// Load all the Vector Data for Each Model ID
	// 
	//===============================================================
	for ( DWORD d = 0; d < dwModelCount+1; d++)
	{
		cModels *pcModel = cModels::Hash_Find( d );
		if(pcModel) 
		{

			sprintf( szCommand, "SELECT * FROM monsters_vector_palettes WHERE dwLinker=%d ORDER BY VectorCount;", pcModel->m_wPaletteVector);

			// Load Palette vector into array

			// Define Temp Variables
			BYTE    bVectorCount; 
			char	readbuff01[5];
			char	readbuff02[3];
			char	readbuff03[3];

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );									
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( bVectorCount ), NULL );	
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff01 , sizeof( readbuff01 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff02 , sizeof( readbuff02 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff03 , sizeof( readbuff03 ), NULL );		

			for ( int intPcount = 0 ; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS && intPcount < pcModel->m_bPaletteChange +1; ++intPcount )
			{
				// Let's scan that text into Hex
				sscanf(readbuff01,"%04x",&pcModel->m_vectorPal[intPcount].m_wNewPalette);
				sscanf(readbuff02,"%02x",&pcModel->m_vectorPal[intPcount].m_ucOffset);
				sscanf(readbuff03,"%02x",&pcModel->m_vectorPal[intPcount].m_ucLength);
			}
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			// Load Texture Vector into Array

			// Define Temp Variables
			char	readbuff04[3];
			char	readbuff05[5];
			char	readbuff06[5];

			sprintf( szCommand, "SELECT * FROM monsters_vector_textures WHERE dwLinker=%d ORDER BY VectorCount;",pcModel->m_wTextureVector);
		
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff04, sizeof( readbuff04 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff05, sizeof( readbuff05 ), NULL );		
	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff06 , sizeof( readbuff06 ), NULL );	

			for ( int intT = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS && intT < pcModel->m_bTextureChange + 1; ++intT )
			{
				//scan the text into hexadecimal
				sscanf(readbuff04,"%02x",&pcModel->m_vectorTex[intT].m_bModelIndex);
				sscanf(readbuff05,"%04x",&pcModel->m_vectorTex[intT].m_wOldTexture);
				sscanf(readbuff06,"%04x",&pcModel->m_vectorTex[intT].m_wNewTexture);
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt ); 
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			// Load Model Vectors into Array

			// Define Temp Variables
			char	readbuff07[3];
			char	readbuff08[5];

			sprintf( szCommand, "SELECT * FROM monsters_vector_models WHERE dwLinker=%d ORDER BY VectorCount;", pcModel->m_wModelVector);

			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 3;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_LONG, &bVectorCount, sizeof( BYTE ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff07, sizeof( readbuff07 ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff08 , sizeof( readbuff08 ), NULL );	

			for ( int intM = 0; (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) && (intM < pcModel->m_bModelChange + 1); ++intM )
			{
				// Scan the Text to HEX
				sscanf(readbuff07,"%02x",&pcModel->m_vectorMod[intM].m_bModelIndex);
				sscanf(readbuff08,"%04x",&pcModel->m_vectorMod[intM].m_wNewModel);
			}

			retcode = SQLCloseCursor( cDatabase::m_hStmt ); 
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
			
			//  Load Model Animations into cAnimation
			sprintf( szCommand, "SELECT * FROM monsters_species_animation WHERE dwSpecies=%d;", pcModel->m_dwSpecies);
		
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	
			retcode = SQLExecute( cDatabase::m_hStmt );
			
			iCol = 4;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wDeath, sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[0], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[1], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[2], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[3], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[4], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[5], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[6], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[7], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[8], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wStance[9], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[0], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[1], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[2], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[3], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[4], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[5], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[6], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[7], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[8], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[9], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wAttack[5], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wIdle[0], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wIdle[1], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wIdle[2], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wIdle[3], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wIdle[4], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wIdle[5], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wReact[0], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wReact[1], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wReact[2], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wReact[3], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wReact[4], sizeof( WORD ), NULL );		
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &pcModel->m_cAnimations.m_wReact[5], sizeof( WORD ), NULL );		

			retcode = SQLFetch( cDatabase::m_hStmt );
			retcode = SQLCloseCursor( cDatabase::m_hStmt ); 
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

//			UpdateConsole( "+" );
		}
	}
}

/**
 *	Loads monsters from the database.
 *
 *	Note: Use ModelNumber to reference pre-loaded model data.
 *		  This should become obsolete once spawn control is developed
 *		  Spawn Generators will then be loaded instead of individual 
 *		  monsters, which will then spawn as needed.
 *
 *	Cube:
 *		Changing this: We will pull monsters for dungeons only from the
 *		monster table. This makes more sense, as dungeons are static spawns.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadMonsters( )
{
	RETCODE		retcode;
	char		szCommand[768];
	char		szMonsters[150];
	DWORD		dwMonsterMax;
	DWORD		dwMonsterCount;

	DWORD		ID;
	char		Name[100];
	char		Description[255];

	cLocation	locMonster;
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	
	DWORD		dwModelNumber;
	cMonStats	cmsStats;
	DWORD		dwRespawn;
	DWORD		dwDecay;
	DWORD		dwChase;
	DWORD		dwInfluence;
	DWORD		dwExp_Value;
	DWORD		dwHealth;
	DWORD		dwStamina;
	DWORD		dwMana;

	sprintf( szCommand, "SELECT MAX(ID), COUNT(ID) FROM monsters;" );

	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );											CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMonsterMax, sizeof( dwMonsterMax ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwMonsterCount, sizeof( dwMonsterCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
	{
		dwMonsterMax = 0;
		dwMonsterCount = 0;
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMonsters, " Loading %d monsters ... ",dwMonsterCount );
	UpdateConsole((char *)szMonsters);

	for ( int iTmp = 1; iTmp < dwMonsterMax + 1; iTmp++) {
		sprintf( szCommand, "SELECT ID,dwModelNumber,strName,strDescription,Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z FROM monsters WHERE ID=%d;", iTmp );
	//	sprintf( szCommand, "SELECT m.dwModelNumber,m.strName,m.strDescription,m.Landblock,m.Position_X,m.Position_Y,m.Position_Z,m.Orientation_W,m.Orientation_X,m.Orientation_Y,m.Orientation_Z,mt.Level,mt.Strength,mt.Endurance,mt.Quickness,mt.Coordination,mt.Focus,mt.Self,mt.Species,mt.Respawn,mt.Decay,mt.Chase,mt.Influence,mt.Health,mt.Stamina,mt.Mana,mt.XP FROM Monsters m LEFT OUTER JOIN monsters_type mt on m.dwModelNumber = mt.Modeldata;" );
		
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
		int iCol = 1;
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &ID, sizeof( ID ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelNumber, sizeof( dwModelNumber ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Name, sizeof( Name ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Description, sizeof( Description ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		// Landblock Data
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locMonster.m_flX, sizeof( &locMonster.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locMonster.m_flY, sizeof( &locMonster.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locMonster.m_flZ, sizeof( &locMonster.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locMonster.m_flA, sizeof( &locMonster.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locMonster.m_flB, sizeof( &locMonster.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locMonster.m_flC, sizeof( &locMonster.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locMonster.m_flW, sizeof( &locMonster.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		if ( SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS ) {
		//	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
			sscanf(dwLandblock,"%08x",&locMonster.m_dwLandBlock);
			sscanf(dwPosX,"%08x",&locMonster.m_flX);
			sscanf(dwPosY,"%08x",&locMonster.m_flY);
			sscanf(dwPosZ,"%08x",&locMonster.m_flZ);
			sscanf(dwOrientW,"%08x",&locMonster.m_flA);
			sscanf(dwOrientX,"%08x",&locMonster.m_flB);
			sscanf(dwOrientY,"%08x",&locMonster.m_flC);
			sscanf(dwOrientZ,"%08x",&locMonster.m_flW);

			cMasterServer::FixName(Name);
		
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

			sprintf( szCommand, "SELECT m.ModelData,m.Level,m.Strength,m.Endurance,m.Quickness,m.Coordination,m.Focus,m.Self,m.Species,m.Respawn,m.Decay,m.Chase,m.Influence,m.Health,m.Stamina,m.Mana,m.XP FROM monsters_type m WHERE m.Modeldata = %d;", dwModelNumber );
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			
			iCol = 2;
			// Monster Level
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwLevel, sizeof( cmsStats.m_dwLevel ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			// Monster Attributes
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwStr, sizeof( cmsStats.m_dwStr ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwEnd, sizeof( cmsStats.m_dwEnd ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwQuick, sizeof( cmsStats.m_dwQuick ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwCoord, sizeof( cmsStats.m_dwCoord ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwFocus, sizeof( cmsStats.m_dwFocus ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwSelf, sizeof( cmsStats.m_dwSelf ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwSpecies, sizeof( cmsStats.m_dwSpecies ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwRespawn, sizeof( dwRespawn ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwDecay, sizeof( dwDecay ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwChase, sizeof( dwChase ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwInfluence, sizeof( dwInfluence ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			// Monster Vitals
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwHealth, sizeof( dwHealth ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwStamina, sizeof( dwStamina), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMana, sizeof( dwMana ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwExp_Value, sizeof( dwExp_Value ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

			if ( SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS )
			{
				cMonster* aMonster = new cMonster(cWorldManager::NewGUID_Object(), dwModelNumber, &locMonster, Name, Description, &cmsStats, dwRespawn, dwDecay, dwChase, dwInfluence, dwExp_Value, dwHealth, dwStamina, dwMana);
				cWorldManager::AddObject( aMonster, TRUE );
			}
		}
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	}
}

/**
 *	Loads monster spawn points from the database.
 *
 *	Author: G70mb2
 */
void cMasterServer::LoadSpawns(WORD LB)
{
	RETCODE		retcode;
	char		szCommand[768];
	char		szMonsters[150];
	char		dwLandblock[9];
	char		dwPosX[9];
	char		dwPosY[9];
	char		dwPosZ[9];
	char		dwOrientW[9];
	char		dwOrientX[9];
	char		dwOrientY[9];
	char		dwOrientZ[9];
	char		Name[100];
	char		Description[255];
	DWORD		dwMonsterCount;
	DWORD		dwModelNumber;
	cMonStats	cmsStats;
	DWORD		dwRespawn;
	DWORD		dwDecay;
	DWORD		dwChase;
	DWORD		dwInfluence;
	DWORD		dwExp_Value;
	DWORD		dwHealth;
	DWORD		dwStamina;
	DWORD		dwMana;
	cLocation	spLoc;

	char buffer [33];
	itoa(LB,buffer,16);

	sprintf( szCommand, "SELECT * FROM monsters WHERE landblock = %s;",buffer);

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMonsterCount, sizeof( dwMonsterCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	
	if( retcode == SQL_NO_DATA )
		dwMonsterCount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
		
	sprintf( szMonsters, " Loading %d monsters ... \n Landblock: %s",dwMonsterCount,buffer );
	UpdateConsole((char *)szMonsters);

	for ( int iTmp = 1; iTmp < dwMonsterCount + 1; iTmp++ )
	{
		sprintf( szCommand, "SELECT ID, m.dwModelNumber,m.strName,m.strDescription,m.Landblock,m.Position_X,m.Position_Y,m.Position_Z,m.Orientation_W,m.Orientation_X,m.Orientation_Y,m.Orientation_Z,mt.dwlevel,mt.dwstr,mt.dwend,mt.dwquick,mt.dwcoord,mt.dwfocus,mt.dwself,mt.dwspecies,mt.dwrespawn,mt.decay,mt.chase,mt.influence,mt.Experience_Value,mt.dwHealth,mt.dwStamina,mt.dwMana FROM Monsters m LEFT OUTER JOIN monstertype mt on m.dwModelNumber = mt.dwModeldata WHERE ID=%d;", iTmp );
//		sprintf( szCommand, "SELECT ID, m.dwModelNumber,m.strName,m.strDescription,m.Landblock,m.Position_X,m.Position_Y,m.Position_Z,m.Orientation_W,m.Orientation_X,m.Orientation_Y,m.Orientation_Z,mt.dwlevel,mt.dwstr,mt.dwend,mt.dwquick,mt.dwcoord,mt.dwfocus,mt.dwself,mt.dwspecies,mt.dwrespawn,mt.decay,mt.chase,mt.influence,mt.Experience_Value,mt.dwHealth,mt.dwStamina,mt.dwMana FROM Monsters m LEFT OUTER JOIN monstertype mt on m.dwModelNumber = mt.dwModeldata;" );
	
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		
		int iCol = 2;

		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelNumber, sizeof( dwModelNumber ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Name, sizeof( Name ), NULL );										CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Description, sizeof( Description ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		// Landblock Data
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwLandblock, sizeof( dwLandblock ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	/*	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &spLoc.m_flX, sizeof( &spLoc.m_flX ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &spLoc.m_flY, sizeof( &spLoc.m_flY ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &spLoc.m_flZ, sizeof( &spLoc.m_flZ ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &spLoc.m_flA, sizeof( &spLoc.m_flA ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &spLoc.m_flB, sizeof( &spLoc.m_flB ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &spLoc.m_flC, sizeof( &spLoc.m_flC ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &spLoc.m_flW, sizeof( &spLoc.m_flW ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	*/	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosX, sizeof( dwPosX ), NULL );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosY, sizeof( dwPosY ), NULL );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwPosZ, sizeof( dwPosZ ), NULL );									CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientW, sizeof( dwOrientW ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientX, sizeof( dwOrientX ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientY, sizeof( dwOrientY ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwOrientZ, sizeof( dwOrientZ ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		// Monster Stats
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwLevel, sizeof( cmsStats.m_dwLevel ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwStr, sizeof( cmsStats.m_dwStr ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwEnd, sizeof( cmsStats.m_dwEnd ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwQuick, sizeof( cmsStats.m_dwQuick ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwCoord, sizeof( cmsStats.m_dwCoord ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwFocus, sizeof( cmsStats.m_dwFocus ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwSelf, sizeof( cmsStats.m_dwSelf ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwSpecies, sizeof( cmsStats.m_dwSpecies ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwRespawn, sizeof( dwRespawn ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwDecay, sizeof( dwDecay ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwChase, sizeof( dwChase ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwInfluence, sizeof( dwInfluence ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwExp_Value, sizeof( dwExp_Value ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwHealth, sizeof( dwHealth ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwStamina, sizeof( dwStamina), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMana, sizeof( dwMana ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		if ( SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS ) {
			sscanf(dwLandblock,"%08x",&spLoc.m_dwLandBlock);
			sscanf(dwPosX,"%08x",&spLoc.m_flX);
			sscanf(dwPosY,"%08x",&spLoc.m_flY);
			sscanf(dwPosZ,"%08x",&spLoc.m_flZ);
			sscanf(dwOrientW,"%08x",&spLoc.m_flA);
			sscanf(dwOrientX,"%08x",&spLoc.m_flB);
			sscanf(dwOrientY,"%08x",&spLoc.m_flC);
			sscanf(dwOrientZ,"%08x",&spLoc.m_flW);

			cMasterServer::FixName(Name);
			cMonster* aMonster = new cMonster(cWorldManager::NewGUID_Object(), dwModelNumber, &spLoc, Name, Description, &cmsStats, dwRespawn, dwDecay, dwChase, dwInfluence, dwExp_Value, dwHealth, dwStamina, dwMana);
			cWorldManager::AddObject( aMonster, TRUE );

	//		UpdateConsole( "+" );
		}
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
}

/**
 *	Spawns a monster from database by name.
 *
 *	Note: Use ModelNumber to reference pre-loaded model data.
 *		  This should become obsolete once spawn control is developed
 *		  Spawn Generators will then be loaded instead of individual 
 *		  monsters, which will then spawn as needed.
 *
 *	@param *szMonster - A pointer to the text representing the monster's name.
 *	@param pcLoc - A cLocation struct describing the avatar's present location.
 *	@param Respawn - A value describing whether the monster should be respawned (0 = No, 1 = Yes).
 *
 *	Author: G70mb2
 */
bool cMasterServer::SpawnMonster( char* szMonster,cLocation pcLoc,DWORD Respawn )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			Name[100];
	char			Description[255];
	cLocation		locMonster;
	DWORD			dwModelNumber;
	cMonStats		cmsStats;
	bool			fSQLOK;
	DWORD			dwRespawn;
	DWORD			dwDecay;
	DWORD			dwChase;
	DWORD			dwInfluence;
	DWORD			dwExp_Value;
	DWORD			dwHealth;
	DWORD			dwStamina;
	DWORD			dwMana;

	fSQLOK	= false;

	sprintf( szCommand, "SELECT * FROM monsters_type WHERE Name='%s';", szMonster );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	UpdateConsole (szMonster);
	int iCol = 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelNumber, sizeof( dwModelNumber ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Name, sizeof( Name ), NULL );										CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Description, sizeof( Description ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwLevel, sizeof( cmsStats.m_dwLevel ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwStr, sizeof( cmsStats.m_dwStr ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwEnd, sizeof( cmsStats.m_dwEnd ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwQuick, sizeof( cmsStats.m_dwQuick ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwCoord, sizeof( cmsStats.m_dwCoord ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwFocus, sizeof( cmsStats.m_dwFocus ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwSelf, sizeof( cmsStats.m_dwSelf ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwSpecies, sizeof( cmsStats.m_dwSpecies ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwRespawn, sizeof( dwRespawn ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwDecay, sizeof( dwDecay ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwChase, sizeof( dwChase ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwInfluence, sizeof( dwInfluence ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwHealth, sizeof( dwHealth ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwStamina, sizeof( dwStamina), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMana, sizeof( dwMana ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol+6, SQL_C_ULONG, &dwExp_Value, sizeof( dwExp_Value ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	if( SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS )
	{
		fSQLOK = true;
		locMonster.m_dwLandBlock = pcLoc.m_dwLandBlock;
		locMonster.m_flA = pcLoc.m_flA;
		locMonster.m_flB = pcLoc.m_flB;
		locMonster.m_flC = pcLoc.m_flC;
		locMonster.m_flW = pcLoc.m_flW;
		locMonster.m_flX = pcLoc.m_flX;
		locMonster.m_flY = pcLoc.m_flY;
		locMonster.m_flZ = pcLoc.m_flZ;

		if(Respawn == 0)
		{
			dwRespawn = 0;
		}
		cMasterServer::FixName(Name);
		cMonster* aMonster = new cMonster(cWorldManager::NewGUID_Object(), dwModelNumber, &locMonster, Name, Description, &cmsStats, dwRespawn, dwDecay, dwChase, dwInfluence, dwExp_Value, dwHealth, dwStamina, dwMana);
		cWorldManager::AddObject( aMonster , true );
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	return fSQLOK;
}

/**
 *	Spawn monster from the database by modelID only.
 *
 *	@param *szMonster - A pointer to the text representing the monster's name.
 *	@param pcLoc - A cLocation struct describing the avatar's present location.
 *	@param dwModelNumber - The model number of the monster to be spawned.
 *	@param dwExp_Value - The experience value of the monster to be spawned.
 *	@param dwHealth - The Health of the monster to be spawned.
 *	@param dwStamina - The Stamina of the monster to be spawned.
 *	@param dwMana - The Mana of the monster to be spawned.
 *
 *	Author: G70mb2
 */
bool cMasterServer::SpawnType( char* szMonster, cLocation pcLoc, DWORD dwModelNumber, DWORD dwExp_Value, DWORD dwHealth, DWORD dwStamina, DWORD dwMana )
{
	cLocation				locMonster;
	cMonStats				cmsStats;
	cmsStats.m_dwSpecies	= 0x40L;
	cmsStats.m_dwLevel		= 10;
	cmsStats.m_dwStr		= 100;
	cmsStats.m_dwEnd		= 100;
	cmsStats.m_dwCoord		= 100;
	cmsStats.m_dwQuick		= 50;
	cmsStats.m_dwFocus		= 50;
	cmsStats.m_dwSelf		= 50;
	dwExp_Value				= 100;
	dwHealth				= 50;
	dwStamina				= 50;
	dwMana					= 25;

	locMonster.m_dwLandBlock = pcLoc.m_dwLandBlock;
	locMonster.m_flA = pcLoc.m_flA;
	locMonster.m_flB = pcLoc.m_flB;
	locMonster.m_flC = pcLoc.m_flC;
	locMonster.m_flW = pcLoc.m_flW;
	locMonster.m_flX = pcLoc.m_flX;
	locMonster.m_flY = pcLoc.m_flY;
	locMonster.m_flZ = pcLoc.m_flZ;

	cMonster* aMonster = new cMonster(cWorldManager::NewGUID_Object(), dwModelNumber, &locMonster, szMonster, "-", &cmsStats, 0, 120, 8, 2,dwExp_Value,dwHealth,dwStamina,dwMana);
	cWorldManager::AddObject( aMonster , true );

	return true;
}

/**
 *	Spawns a monster from the database and saves a copy into the auto-load database.
 *
 *	@param *szMonster - A pointer to the text representing the monster's name.
 *	@param pcLoc - A cLocation struct describing the avatar's present location.
 *	@param bFacing - A boolean value for whether the monster should be facing the avatar.
 *	@param bOverride - A boolean value for whether the monster's default respawn, decay, and influence values should be overwriten.
 *	@param Decay - 
 *	@param Chase - 
 *	@param Influence - 
 *
 *	Author: G70mb2
 */
bool cMasterServer::SpawnSave( char* szMonster, cLocation pcLoc, bool bFacing, bool bOverride, DWORD Respawn, DWORD Decay, DWORD Chase, DWORD Influence )
{
	RETCODE			retcode;
	char			szCommand[512];
	char			Name[100];
	char			Description[255];
	cLocation		locMonster;
	DWORD			dwModelNumber;
	cMonStats		cmsStats;
	bool			fSQLOK;
	DWORD			dwRespawn;
	DWORD			dwDecay;
	DWORD			dwChase;
	DWORD			dwInfluence;
	DWORD			dwExp_Value;
	DWORD			dwHealth;
	DWORD			dwStamina;
	DWORD			dwMana;

	cMonster*		aMonster;

	fSQLOK	= false;

	sprintf( szCommand, "SELECT * FROM monsters_type WHERE Name='%s';", szMonster );
	
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	int iCol = 2;
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwModelNumber, sizeof( dwModelNumber ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Name, sizeof( Name ), NULL );										CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, Description, sizeof( Description ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwLevel, sizeof( cmsStats.m_dwLevel ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwStr, sizeof( cmsStats.m_dwStr ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwEnd, sizeof( cmsStats.m_dwEnd ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwQuick, sizeof( cmsStats.m_dwQuick ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwCoord, sizeof( cmsStats.m_dwCoord ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwFocus, sizeof( cmsStats.m_dwFocus ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwSelf, sizeof( cmsStats.m_dwSelf ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &cmsStats.m_dwSpecies, sizeof( cmsStats.m_dwSpecies ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwRespawn, sizeof( dwRespawn ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwDecay, sizeof( dwDecay ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwChase, sizeof( dwChase ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwInfluence, sizeof( dwInfluence ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwExp_Value, sizeof( dwExp_Value ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwHealth, sizeof( dwHealth ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwStamina, sizeof( dwStamina), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwMana, sizeof( dwMana ), NULL );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	if( SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS )
	{
		fSQLOK = true;
		if(bFacing == true)
		{
			locMonster.m_dwLandBlock = pcLoc.m_dwLandBlock;
			locMonster.m_flA = pcLoc.m_flA;
			locMonster.m_flB = pcLoc.m_flB;
			locMonster.m_flC = pcLoc.m_flC;
			locMonster.m_flW = pcLoc.m_flW;
			locMonster.m_flX = pcLoc.m_flX;
			locMonster.m_flY = pcLoc.m_flY;
			locMonster.m_flZ = pcLoc.m_flZ;
		}
		else
		{
			locMonster.m_dwLandBlock = pcLoc.m_dwLandBlock;
			locMonster.m_flW = -pcLoc.m_flA;
			locMonster.m_flB = pcLoc.m_flB;
			locMonster.m_flC = pcLoc.m_flC;
			locMonster.m_flA = pcLoc.m_flW;
			locMonster.m_flX = pcLoc.m_flX;
			locMonster.m_flY = pcLoc.m_flY;
			locMonster.m_flZ = pcLoc.m_flZ;
		}

		if(bOverride == true)
		{
			dwRespawn = Respawn;
			dwDecay = Decay;
			dwChase = Chase;
			dwInfluence = Influence;
		}
		cMasterServer::FixName(Name);
		aMonster = new cMonster(cWorldManager::NewGUID_Object(), dwModelNumber, &locMonster, Name, Description, &cmsStats, dwRespawn, dwDecay, dwChase, dwInfluence, dwExp_Value, dwHealth, dwStamina, dwMana);
		cWorldManager::AddObject( aMonster , true );
	}

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	if (fSQLOK == true)
	{
		DWORD flX;
		DWORD flY;
		DWORD flZ;
		DWORD flA;
		DWORD flB;
		DWORD flC;
		DWORD flW;

		// floating point to 32-bit hexadecimal
		flX = cDatabase::Float2Hex(locMonster.m_flX);
		flY = cDatabase::Float2Hex(locMonster.m_flY);
		flZ = cDatabase::Float2Hex(locMonster.m_flZ);
		flA = cDatabase::Float2Hex(locMonster.m_flA);
		flB = cDatabase::Float2Hex(locMonster.m_flB);
		flC = cDatabase::Float2Hex(locMonster.m_flC);
		flW = cDatabase::Float2Hex(locMonster.m_flW);

		BOOL fVerify = TRUE;
		char Command[200];

		sprintf( Command, "INSERT INTO Monsters (dwGUID, dwModelNumber, strName, strDescription, Landblock, Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z) VALUES (%lu,%d,'%s', '%s','%08x','%08x','%08x','%08x','%08x','%08x','%08x','%08x');",aMonster->GetGUID(),aMonster->m_dwMonsterModelID,szMonster,Description,locMonster.m_dwLandBlock,flX,flY,flZ,flA,flB,flC,flW);

		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)Command, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
		
		if( retcode == SQL_ERROR )
			fVerify = FALSE;
		
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );							CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	
		if(fVerify == false)
		{
			UpdateConsole( " <SQL> Monster creation failed!\r\n" );
			fSQLOK = false;
		}
		else
		{
			UpdateConsole( " <SQL> Creating monster entry ...\r\n" );
			fSQLOK = true;
		}
	}
	else
	{
		fSQLOK = false;
	}
	return fSQLOK;
}

/**
 *	Spawns an NPC from the database and saves a copy into the auto-load database.
 *
 *	@param *szNPC - A pointer to the text representing the NPC's name.
 *	@param pcLoc - A cLocation struct describing the avatar's present location.
 *	@param bFacing - A boolean value for whether the NPC should be facing the avatar.
 *
 *	Author: G70mb2
 */
/* Disabling this...
bool cMasterServer::NPC_Save( char* szNPC, cLocation pcLoc, bool bFacing )
{
	RETCODE			retcode;
	bool			fSQLOK;
	fSQLOK	= false;
	cLocation		locNPC;

	if(bFacing == true)
	{
		locNPC.m_dwLandBlock = pcLoc.m_dwLandBlock;
		locNPC.m_flA = pcLoc.m_flA;
		locNPC.m_flB = pcLoc.m_flB;
		locNPC.m_flC = pcLoc.m_flC;
		locNPC.m_flW = pcLoc.m_flW;
		locNPC.m_flX = pcLoc.m_flX;
		locNPC.m_flY = pcLoc.m_flY;
		locNPC.m_flZ = pcLoc.m_flZ;
	}
	else
	{
		locNPC.m_dwLandBlock = pcLoc.m_dwLandBlock;
		locNPC.m_flW = -pcLoc.m_flA;
		locNPC.m_flB = pcLoc.m_flB;
		locNPC.m_flC = pcLoc.m_flC;
		locNPC.m_flA = pcLoc.m_flW;
		locNPC.m_flX = pcLoc.m_flX;
		locNPC.m_flY = pcLoc.m_flY;
		locNPC.m_flZ = pcLoc.m_flZ;
	}

	cNPC* npc = new cNPC(szNPC, 0x1, &locNPC, 0);	
	
	npc->SetMode(NPCMODE_SINGLE);	 // Message Mode
	npc->SetNumMessages(1);	 // Number of Messages
	npc->SetModel(0); // Use Default NPC Model

	// Store Blank Messages
	npc->SetString("I can be Customized by Editing the NPCs table in the Database",0);
	npc->SetString(" ",1);
	npc->SetString(" ",2);
	npc->SetString(" ",3);
	npc->SetString(" ",4);
	npc->SetString(" ",5);
	npc->SetString(" ",6);
	npc->SetString(" ",7);
	npc->SetString(" ",8);
	npc->SetString(" ",9);

	cWorldManager::AddObject( npc , true );

	BOOL fVerify = true;
	char Command[300];
	
	cMasterServer::FixName(szNPC);

	sprintf( Command, "INSERT INTO NPCs (Name, Gender, wModelNum, Message_Type,Message_Count,Message_1,Landblock, fl_X,fl_Y,fl_Z,fl_Heading,Unknown_1,Unknown_2,Heading_2 ) VALUES ('%s','m',88,'SINGLE', 1, 'I can be Customized by Editing the NPCs table in the Database', %lu,%f,%f,%f,%f,%f,%f,%f);",cWorldManager::NewGUID_Object(),szNPC,locNPC.m_dwLandBlock,locNPC.m_flX,locNPC.m_flY,locNPC.m_flZ,locNPC.m_flA,locNPC.m_flB,locNPC.m_flC,locNPC.m_flW );

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)Command, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
	
	if( retcode == SQL_ERROR )
	{
		fVerify = false;
	}

	if(fVerify == false)
	{
		UpdateConsole( " <SQL> Creating NPC failed!\r\n" );
		fSQLOK = false;
	}
	else
	{
		UpdateConsole( " <SQL> NPC created.\r\n" );
		fSQLOK = true;
	}

	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

	return fSQLOK;
}
*/