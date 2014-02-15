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
 *	@file CharacterServer.h
 */

#ifndef __CHARACTERSERVER_H
#define __CHARACTERSERVER_H

#include "uas.h"

class cCharacterServer
{
	friend class cMasterServer;

public:
	static DWORD m_dwClientCount;	
	static DWORD ReturnPadOffset( DWORD dwLength );
	static void Initialize( short nPort, WORD wLogicalID, DWORD dwSendCRCSeed, DWORD dwRecvCRCSeed )
	{
		UpdateConsole( "\r\n Starting Character Server ... " );

		SOCKADDR_IN	saSockAddr;
		saSockAddr.sin_family		= AF_INET;
		saSockAddr.sin_port			= htons( nPort );
		saSockAddr.sin_addr.s_addr	= INADDR_ANY;

		m_wLogicalID	= wLogicalID;
		m_dwSendCRCSeed = dwSendCRCSeed;
		m_dwRecvCRCSeed = dwRecvCRCSeed;

		m_Socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if ( !m_Socket )
		{
			UpdateConsole( "socket( ) failed!\r\n" );
			return ;
		}

		if ( bind( m_Socket, (struct sockaddr *)&saSockAddr, sizeof( SOCKADDR_IN ) ) )
		{
			UpdateConsole( "bind( ) failed!\r\n" );
			return ;
		}

		UpdateConsole( "success.\r\n" );
	}

	static void Halt( )
	{
		closesocket( m_Socket );
		UpdateConsole( " Character Server: closing socket ...\r\n" );
	}
	
protected:
	static DWORD	m_dwSendCRCSeed;
	static DWORD	m_dwRecvCRCSeed;
	static WORD		m_wLogicalID;
	static SOCKET	m_Socket;
};

#endif	// #ifndef __CHARACTERSERVER_H