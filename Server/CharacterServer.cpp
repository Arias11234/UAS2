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
 *	@file CharacterServer.cpp
 *	Handles the CharacterServer.
 *
 *	CharacterServer handles interactions with clients outside of the server game world.
 *	Actions include account verification, providing the character list, the creation and deletion
 *	of characters, and the connection and disconnection of clients.
 */

#include <sstream>

#include "Client.h"
#include "CharacterServer.h"
#include "MasterServer.h"
#include "RecvPacket.h"
#include "VersionNo.h"

// Initialize static members
//================================================
DWORD	cCharacterServer::m_dwSendCRCSeed;
DWORD	cCharacterServer::m_dwRecvCRCSeed;
WORD	cCharacterServer::m_wLogicalID;
SOCKET	cCharacterServer::m_Socket;
DWORD	cCharacterServer::m_dwClientCount;
//DWORD   cCharacterServer::ReturnPadOffset( DWORD dwLength );
DWORD	dwClientCount;
//================================================

static BYTE PacketTwo[122] = {  // Nov Patch
	0x01, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 
	0x67, 0x4A, 0x83, 0x6A, 
	0x0B, 0x00, 0x00, 0x00, 0x66, 0x00, 0x00,0x00, //0x01, 0x00,
    
	0x52, 0x5B, 0x95, 0x5B,
	0xC9, 0x9A, 0x97, 0x41,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0xB2, 0x00, 0x0C, 0x00, 
	// Byte 41
	0x00, 0x00, 0x32, 0x30, 0x30, 0x34, 0x2E, 0x30, 0x31, 0x2E, 0x30, 0x30, 0x31, 0x00, 0x00, 0x20, 0x00, 0x20, // 58 bytes
	// Keep below from Dec Client
	0x3C, 0x00, 0x00, 0x00, 0x88, 0x77, 0x66, 0x33, 0x08, 0x00, 0x00, 0x00,	
	0xFB, 0x63, 0xC5, 0x08, 0x16, 0xC7, 0x80, 0x31, 0xFF, 0x43,	0xE5, 0xFE, 
	0x8A, 0xF3, 0xE3, 0x69, 0x23, 0xCF, 0xF8, 0x82, 0xB9, 0xC4, 0x59, 0x80, 
	0x99, 0x30,	0xB6, 0x08, 0xE5, 0xC2, 0x80, 0xDD, 0xBB, 0xAD, 0xCD, 0xAD, 
	0x03, 0x00, 0x00, 0x00, 0xF8, 0xCA,	0xEF, 0x74, 0x54, 0x9E, 0xFC, 0x4B, 
	0x85, 0x48, 0xDF, 0xBB,  	// 122 bytes
};

static BYTE PacketThree[250] = {
	0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0xD7, 0xA2, 0xCD, 0x6B, 0x0B, 0x00, 0x00, 0x00,   //(____@______k____)
	0xE6, 0x00, 0x00, 0x00, 0xE3, 0x5B, 0x4C, 0x6F, 0xE6, 0xD4, 0x8F, 0x41, 
	0x00, 0x00, 0x00, 0x00,   //(_____[Lo___A____)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //(________________)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                       //(__________)
};

static BYTE UserName[44] = { // This is the tail of the Char list used in the June+ Clients
	0xEF,0xBE,0xEF,0xBE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,
};

static char szClientVersion[12] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static char szBadClientVersion[12] = {
	0x32, 0x30, 0x30, 0x34, 0x2E, 0x30, 0x31, 0x2E, 0x30, 0x30, 0x31, 0x00
};

static byte Invalid2[24] = {
	0x19, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x5F, 0x72, 0x6B, 0xBD, 
	0x15, 0x00, 0x7A, 0x02, 0x04, 0x00, 0x00, 0x00, 0x4C, 0x01, 0x00, 0x00,
};

std::string aMonth[12] = { "January","February","March","April","May","June","July","August","September","October","November","December"};

/**
 *	Processes Character Server packets
 *
 *	This function is called when the client sends a general Character Server message.
 *	These encompass the actions performed by a client when outside of the server game world.
 */
void cClient::ProcessPacket_CS( cRecvPacket *pcRecvPacket )
{
	m_dwLastRecvTime = timeGetTime( );

	switch ( pcRecvPacket->GetFlags( ) )
	{
		//Client logout
		case 0x00000020:
		{
			cMasterServer::DisconnectClient( this );
			if(cMasterServer::m_dwNumUsers > 0)
			{
				cMasterServer::m_dwNumUsers--;
			}
			return ;
		}
		case 0x000000004:
		{
			DWORD dwLastGoodSeq;

			m_PacketPipe_CS.m_wTime = pcRecvPacket->GetTime( );
			pcRecvPacket->CopyPayload( 0, &dwLastGoodSeq, 4 );
			
			m_PacketPipe_CS.ClearSentPackets( dwLastGoodSeq );
			return ;
		}
		case 0x00100000:
		case 0x00200200:
		case 0x00000200:
		{
			m_PacketPipe_CS.m_dwRecvSequence	= pcRecvPacket->GetSequence( );
			m_PacketPipe_CS.m_wTime				= pcRecvPacket->GetTime( );

			if ( pcRecvPacket->GetFlags( ) == 0x00100000 )
			{
				m_PacketPipe_CS.ReturnPing( m_saSockAddr );
				return ;
			}

			break;	// case 0x00100000, case 0x00200200, and case 0x00000200
		}
	}
	#ifdef _DEBUG
		char szPacketA[100];
		sprintf( szPacketA, "Character Server Packet: %08x ",pcRecvPacket->GetFlags( ));
		cMasterServer::WriteToFile(szPacketA);
		//cMasterServer::ServerMessage( ColorGreen,this,(char *)szPacketA);
	#endif

	switch ( m_bCharState )
	{
		case 1:
		{
			if ( pcRecvPacket->GetFlags( ) == 0x00000000 )
			{
				m_bCharState = 2;
				bool bClientOk = false;
				 
				CopyMemory( &szClientVersion[0], pcRecvPacket->GetPayload( 06 ), 12 ); // Connecting client version 

				//If the client is of the correct version
				if((szClientVersion[0] == 0x32)&&(szClientVersion[1] == 0x30)&&(szClientVersion[2] == 0x30)&&(szClientVersion[3] == 0x34))
				{
					bClientOk = true;
				}

				char szLoginInfo[45];
				char szPasswordInfo[45];
				WORD wLength;

				CopyMemory(&wLength, pcRecvPacket->GetPayload(32),2);

				if(szClientVersion[3] == '4')
				{
					// <-- Post 012704 Clients
					CopyMemory( szLoginInfo, pcRecvPacket->GetPayload( 34 ), wLength ); 
				}			
				else if(szClientVersion[0] == '5')
				{
					// Sentinel Client
					CopyMemory( szLoginInfo, pcRecvPacket->GetPayload( 76 ), 44 );
				}
				else
				{
					//<-- Pre 012704 Clients
					CopyMemory( szLoginInfo, pcRecvPacket->GetPayload( 74 ), 44 );	// Dec-> (80), 44); June-> (74), 44);
				}

				szLoginInfo[wLength] = '\0';
				char* pszSep = strchr(szLoginInfo, (int)':');
				
				char szIP[32];
				cMasterServer::FormatIP_Port( m_saSockAddr, szIP );

				//Display console text information for a client login attempt
				m_wAccountNameLength = pszSep-szLoginInfo;
				CopyMemory( m_szAccountName, szLoginInfo,m_wAccountNameLength );
				m_szAccountName [m_wAccountNameLength] = '\0';
				UpdateConsole(	" Client login attempt:\r\n"
								"      User: %s\r\n"
								"      IP: %s\r\n", m_szAccountName,szIP);

				//If no password is provided
				if(pszSep == NULL)
				{
					UpdateConsole( "     Invalid login information from IP %s.\r\n ", szIP );
					cClient::Hash_Remove( this );
					break;
				}
				else
				{
					*pszSep = '\0';
					++pszSep;
					CopyMemory( &UserName[0], szLoginInfo, sizeof(szLoginInfo) );  // Added for June Client
					cMasterServer::m_dwNumUsers++;
				}
				if( bClientOk == true)
				{
					CopyMemory( szPasswordInfo, pszSep,wLength );

					if ( !cDatabase::VerifyAccount( szLoginInfo, szPasswordInfo, m_dwAccountID, TRUE ))
					{
						UpdateConsole( "      <SQL> No matching account found.\r\n" );
						bClientOk = false;
						//cClient::Hash_Remove( this );
						//cMasterServer::DeleteClient( m_saSockAddr );
						//break;	// case 1
					}
					else
					{
						UpdateConsole( "      <SQL> Account verified.\r\n" );
					}
				}
				else
				{
						UpdateConsole( "      Invalid client connection attempt.\r\n" );
				}
							
				m_dwLastRecvTime = 0;

				//If the client is of the correct version
				if(bClientOk == true)
				{
					CopyMemory( &PacketTwo[42],szClientVersion,12);
					m_PacketPipe_CS.CalcCRC( PacketTwo, 102 );
					sendto( m_PacketPipe_CS.m_Socket, (char *)PacketTwo, sizeof( PacketTwo ), NULL, (SOCKADDR *)&m_saSockAddr, sizeof( SOCKADDR ) );
				}
				else
				{
					CopyMemory( &PacketTwo[42],szBadClientVersion,12);
					m_PacketPipe_CS.CalcCRC( PacketTwo, 102 );
					sendto( m_PacketPipe_CS.m_Socket, (char *)PacketTwo, sizeof( PacketTwo ), NULL, (SOCKADDR *)&m_saSockAddr, sizeof( SOCKADDR ) );
					cClient::Hash_Remove( this );
				}
			}
			break;	//case 1
		}

		case 2:
		{
			if ( pcRecvPacket->GetFlags( ) == 0x00000080 )
			{
				m_bCharState = 3;

				m_PacketPipe_CS.CalcCRC( PacketThree, 230 );
				for ( int i = 0; i < 10; ++i )
					sendto( m_PacketPipe_CS.m_Socket, (char *)PacketThree, sizeof( PacketThree ), NULL, (SOCKADDR *)&m_saSockAddr, sizeof( SOCKADDR ) );
			}
			break;	//case 2
		}

		case 3:
		{
			if ( pcRecvPacket->GetFlags( ) == 0x00000040 )
			{
				m_bCharState = 4;
				SendPacketF7B8( );
				cDatabase::LoadAvatarList( m_dwAccountID, m_AvatarList );
				SendAvatarList( );
				SendMOTD( );
			}
			break;	//case 3
		}
		
		case 4:
		{ 
			if ( pcRecvPacket->GetFlags( ) & 0x00000200 )
			{
				DWORD dwMsgId;
				pcRecvPacket->CopyPayload( 16, &dwMsgId, sizeof( DWORD ) );

				switch(dwMsgId)
				{
					case 0xF7C8L:
							SendPacketF7C7( );
							break;
					
					//Avatar deletion
					case 0xF655:
						{
							//AvatarDeleteMessage *adm = (AvatarDeleteMessage*)pcRecvPacket->GetPayload( 16 );
							AvatarDeleteMessage adm;
							pcRecvPacket->CopyPayload(16, &adm, 6 );
							pcRecvPacket->CopyPayload( 16 + 6, adm.szAccountName, adm.wNameLength + 1);
							pcRecvPacket->CopyPayload( 16 + 6 + adm.wNameLength + cCharacterServer::ReturnPadOffset(adm.wNameLength), &adm.dwSlot, 8 );

							cDatabase::DeleteAvatar( (m_AvatarList.begin() + adm.dwSlot)->m_dwGUID );

							m_AvatarList.erase( m_AvatarList.begin() + adm.dwSlot );

							AddPacket( CHAR_SERVER, (unsigned char*)&dwMsgId, sizeof( dwMsgId ), 4 );
							
							SendDeleteAck( );
							SendAvatarList( );
							break;
						}
					
					//Character creation
					case 0xF656L:
						{
							CreateCharacterMessage ccm;
							pcRecvPacket->CopyPayload( 16, &ccm, 6 );
							//ccm.szAccountName = new char[ccm.wLengthOfAccountName + 1];
							pcRecvPacket->CopyPayload( 16 + 6, ccm.szAccountName, ccm.wLengthOfAccountName + 1);

							pcRecvPacket->CopyPayload( 16 + 6 + ccm.wLengthOfAccountName + cCharacterServer::ReturnPadOffset(ccm.wLengthOfAccountName), &ccm.dwValOne, 152 );

							pcRecvPacket->CopyPayload( 16 + 6 + ccm.wLengthOfAccountName + cCharacterServer::ReturnPadOffset(ccm.wLengthOfAccountName) + 152, &ccm.dwNumSkills, 4 );
							pcRecvPacket->CopyPayload( 16 + 6 + ccm.wLengthOfAccountName + cCharacterServer::ReturnPadOffset(ccm.wLengthOfAccountName) + 152 + 4, &ccm.dwSkillStatus, 4 * ccm.dwNumSkills );

							pcRecvPacket->CopyPayload( 16 + 6 + ccm.wLengthOfAccountName + cCharacterServer::ReturnPadOffset(ccm.wLengthOfAccountName) + 152 + 4 + (4 * ccm.dwNumSkills), &ccm.wNameLength, 2 );
							pcRecvPacket->CopyPayload( 16 + 6 + ccm.wLengthOfAccountName + cCharacterServer::ReturnPadOffset(ccm.wLengthOfAccountName) + 152 + 4 + (4 * ccm.dwNumSkills) + 2, ccm.szName, ccm.wNameLength);
							ccm.szName[ccm.wNameLength] = '\0';	//fix the name from name padding

							DWORD Test;
							Test = cCharacterServer::ReturnPadOffset(ccm.wLengthOfAccountName);

							//Cubem0j0:  Test to see what is returned for character name as this does not appear to work.
							FILE* pFile = cMasterServer::pFile;

							pFile = fopen( "server.log", "wa" );
							setbuf(pFile,NULL);
							cMasterServer::WriteToFile(ccm.szName);
							
							//End test

							pcRecvPacket->CopyPayload( ((cFragmentHeader*)(pcRecvPacket->m_bData+20))->m_wFragmentLength - 16, &ccm.dwUnk4, 16);

							cAvatarList cAL;

							if( cDatabase::CreateAvatar( m_dwAccountID, ccm, cAL.m_dwGUID ) )	//Create avatar
							{
								cMessage cMsg;
								cMsg << 0xF7B0L << 0L << 0L << 0xF643L << 1L << cAL.m_dwGUID << ccm.szName << 0L;
								AddPacket( CHAR_SERVER, cMsg, 4);

								cAL.m_strName = ccm.szName;
								m_AvatarList.push_back(cAL);

								SendAvatarList( );
							}
							else
							{
								cMessage cMsg;
								cMsg << 0xF7B0L << 0L << 0L << 0xF643L << 3L << 0xBEEFBEEF << ccm.szName << 0L;
								AddPacket( CHAR_SERVER, cMsg, 4);
							
							}

							//SAFEDELETE_ARRAY( ccm.szAccountName )
							//SAFEDELETE_ARRAY( ccm.szName )

							break;
						}
					
					//Avatar creation
					case 0xF657L:
							cMasterServer::CreateNewAvatar( &m_pcAvatar, ((DWORD*)&(*pcRecvPacket)[0])[0x0A] );
							m_pcAvatar->m_wNumLogins = 0x3C;
							m_pcAvatar->m_wModelSequenceType = 0xC6;
							m_pcAvatar->m_wModelSequence = 0x0;
							SendAddress( g_szLocalIP, g_nWorldPort );
							m_bCharState = 6;
							break;
				}
			}
			break;	// case 4
		}

		case 6:
		{
			if ( pcRecvPacket->GetFlags( ) == 0x00020000 )
				m_bCharState = 7;
			else
				SendAddress( g_szLocalIP, g_nWorldPort );

			break;	// case 6
		}

		case 7:
		{
			break;	// case 7
		}

		//Client connection (to the server page)
		case 8:
		{
			m_bCharState = 4;

			SendAvatarList( );	//Send avatar list
			SendMOTD( );		//Send message of the day

			DWORD dwEnd3D = 0xF653;
			AddPacket( CHAR_SERVER, (BYTE *)&dwEnd3D, 4, 4 );
			
			if(cCharacterServer::m_dwClientCount > 0)
			{
				cCharacterServer::m_dwClientCount--;
			}
			
			break;	// case 8
		}
	}
}

/**
 *	Sends the client its list of avatars on the server
 *
 *	This function is called whenever a client connects to a server
 *	Returns a server message to the client.
 */
void cClient::SendAvatarList( )
{
	cMessage cMsg;
	cMsg << 0xF658L << 0x0000L << m_AvatarList.size();
	
	//Iterate through the client's avatar list
	for( std::vector<cAvatarList>::iterator ical = m_AvatarList.begin(); ical != m_AvatarList.end(); ical++ )
	{
		cMsg << ical->m_dwGUID << ical->m_strName.c_str( );
		cMsg.pasteAlign(4);
		cMsg << 0L;
	}
	cMsg << 0L << 5L; //Dec Client ends here

	//June Client addition starts here
	for (int ib = 0; ib < 45; ib++)
	{
		if ((UserName[ib]) == 0)
		{
			break;
		}

	}
	cMsg << WORD(ib);
	cMsg.pasteData(UserName,ib);
	//for (int ia = 0; ia < (45 - ib); ia++)
	//{
	//	cMsg << BYTE(0x00);
	//}
	cMsg.pasteAlign(4);
	cMsg << DWORD(0x0L);
	//June Client ends here

	AddPacket( CHAR_SERVER, cMsg, 4 );
}

void cClient::SendDeleteAck( )
{
	cMessage cAckMsg;
	cAckMsg << 0xF655L << 0x00108F04;
	AddPacket( CHAR_SERVER, cAckMsg, 4 );
}

/**
 *	Sends the client the server Message of the Day
 *
 *	This function is called whenever a client connects to a server
 *	Returns a server message to the client.
 */
void cClient::SendMOTD( )
{
	FILE *pcMOTD = fopen( "Server.motd","rt" );
	std::stringstream strstrm;
	std::stringstream strstrm2;
	SYSTEMTIME st;

	GetSystemTime( &st );
	
	char str[20] ={0,};
	CopyMemory( &str[0], &STRFILEVER, sizeof(STRFILEVER) );

	for(int a = 0;a<strlen(str);a++)
	{
		if(str[a] == ',')
		{
			str[a] = '.';
		}
	}
	
	//The Message of the Day text
	strstrm << "Currently " << cMasterServer::m_UserCount << " user(s) playing.\n";//cCharacterServer::m_dwClientCount << " clients connected.\n";
	strstrm2 << "You are in the World of " << cMasterServer::m_szServerName << ".\n";
	strstrm2 << "\nWelcome to Asheron's Call!\n\n";
	strstrm2 << aMonth[st.wMonth -1] << " " << st.wDay << ", " << st.wYear << "\n";
	strstrm2 << "=================\n";
	
	strstrm2 << " This emulator is powered by\n";
    strstrm2 << " UAS2: http://uastwo.googlecode.com\n";
	strstrm2 << " Debug Build Copy\n";
	strstrm2 << " Server Version " << SERVERVERSION <<"  v";
	strstrm2 << str << "\n";
	strstrm2 << "=================\n";
	strstrm2 << " Developers:\n";
	strstrm2 << "     Cubem0j0\n";
	strstrm2 << "     eLeM\n";
	strstrm2 << "     agentsparrow\n";
	strstrm2 << "=================\n";
	strstrm2 << " Special thanks to:\n";
	strstrm2 << "     Johnnyjpg -> For webspace to host our forum!\n";
	strstrm2 << "     k109          -> For project management assistance.\n";
	strstrm2 << "     Paaa          -> For helping to test early releases.\n";
	strstrm2 << "     And all of the fans at http://forum.johnnyjpg.com!\n";
	strstrm2 << "=================\n";

	if ( pcMOTD )
	{
		char buff[65536];

		while ( !feof( pcMOTD ) )
		{
			fgets( buff, 65535, pcMOTD );
			strstrm << buff;
		}

		fclose( pcMOTD );
	}
			
	cMessage cMsg;
    cMsg << 0xF65AL << strstrm.str( ).c_str( ) << strstrm2.str( ).c_str( );
	AddPacket( CHAR_SERVER, cMsg, 4 );
}

void cClient::SendAddress( char *szAddr, WORD wPort )
{
	BYTE pbSendBuffer[0x60];
	SOCKADDR_IN *saSockAddr = (SOCKADDR_IN *)&pbSendBuffer[0x14];
	
	cTransportHeader *pcTH	= reinterpret_cast< cTransportHeader * >( pbSendBuffer );
	pcTH->m_dwSequence		= 0;
	pcTH->m_dwFlags			= 0x00020000;
	pcTH->m_wLogicalID		= m_PacketPipe_CS.m_wLogicalID;
	pcTH->m_wTotalSize		= 0x10;
	pcTH->m_wTime			= 0;
	pcTH->m_wTable			= 0;

	saSockAddr->sin_family		= AF_INET;
	saSockAddr->sin_port		= htons( wPort );
	saSockAddr->sin_addr.s_addr = inet_addr( szAddr );
	ZeroMemory( &saSockAddr->sin_zero, 8 );

	m_PacketPipe_CS.CalcCRC( pbSendBuffer, 16 );

	sendto( m_PacketPipe_CS.m_Socket, (char *)pbSendBuffer, 36, NULL, (SOCKADDR *)&m_saSockAddr, sizeof( SOCKADDR ) );
}

/**
 *	Pads messages to the DWORD boundary.
 *
 *	Used for variable-length (text/char[]) fields.
 *	The client expects the next variable to start at the next whole DWORD (4 byte value)
 *	The variable length field is zero-padded until this boundary is reached.
 *
 *	@param dwLength - The length of the variable-length field.
 *
 *	@return dwOffset - The BYTE value of padding required.
 */
DWORD cCharacterServer::ReturnPadOffset( DWORD dwLength )
{
	DWORD dwOffset;
	int intTest;

	intTest = ((dwLength + 2) % 4);

	switch(intTest)
	{
		case 0:
			{
				dwOffset = 0;
				break;
			}
		case 1:
			{
				dwOffset = 3;
				break;
			}
		case 2:
			{
				dwOffset = 2;
				break;
			}
		case 3:
			{
				dwOffset = 1;
				break;
			}
		default:
			{
				dwOffset = 0;
				break;
			}
	}
	return dwOffset;	// Returns DWORD alignment padding amount
}