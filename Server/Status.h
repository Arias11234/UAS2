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
 *	@file Status.h
 */

#if !defined(AFX_STATUS_H__28A3DF38_A042_4F48_8966_9973B6803166__INCLUDED_)
#define AFX_STATUS_H__28A3DF38_A042_4F48_8966_9973B6803166__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "uas.h"

class CStatus  
{
public:
	bool fPrivate;
	static void		SendHTTP(char* strOutput);
	static void		ServerOffline( );
	static void		ServerShutdown( );
	static void		ServerStart( );
	static void		ServerLoad( );
	static void		UpDate( );
	static void		Clear( );
	static DWORD	m_dwMax;
	CStatus();
	virtual ~CStatus();

	static char		m_strHost[16];
	static char		m_strHTTP[500];
	static char		m_strOutput[500];
	static char		m_strSend[1024];
	static int		m_fPrivate;
	static char		m_mText[51];
	static DWORD	m_sTTE;
	static DWORD	m_cAvg;
	static char		m_cVersion[10];
	static char		m_cVer[31];
	static int		m_sPort;
	static char		m_sKey[7];
	static char		m_sSer[31];
	static DWORD	m_sID;
	static byte		m_nStat;
	static SOCKET	m_Socket;
	static WSADATA	m_wsaData;
	static SOCKET	m_SendSocket;
	//
	static byte		m_bServer[4];
	static int		m_ServerPort;
	static unsigned long	m_ulLastUpdate;	
};

#endif // !defined(AFX_STATUS_H__28A3DF38_A042_4F48_8966_9973B6803166__INCLUDED_)