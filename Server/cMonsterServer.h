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
 *	@file cMonsterServer.h
 */

#if !defined(AFX_CMONSTERSERVER_H__84170BF9_CB4B_4559_B900_790678A46F28__INCLUDED_)
#define AFX_CMONSTERSERVER_H__84170BF9_CB4B_4559_B900_790678A46F28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <algorithm>
#include "math.h"
#include "Shared.h"
#include "Object.h"

class cMonsterServer  
{
	friend class cMasterServer;

public:
	static void Initialize( )
	{
		UpdateConsole(" Starting Monster Server ... " );
		
		time(&NextActionTime + ACTION_DELAY);

		UpdateConsole("success.\r\n\r\n");
	}

	static void Halt( )
	{
		UpdateConsole(" Monster Server: shutting down ...\r\n");
	}

	static void			ProcessAction	( DWORD dwMonGUID, DWORD dwEvent, DWORD dwEngaged, DWORD dwTarget = 0x0L );
	static void			ProcessPetAction	( DWORD dwMonGUID, DWORD dwEvent, DWORD dwEngaged, DWORD dwTarget = 0x0L, DWORD dwOwner = 0x0L );
	static void			ActionCheck		( );
	static bool			AddMonster		( DWORD dwGUID );
	static bool			RemoveMonster	( DWORD dwGUID );
	static void			Underattack		( DWORD dwGUID );	
	
	//Variables
	static sMonsters	Monster[MAX_MONSTERS];
	static time_t		longtime;
	static time_t		NextActionTime;

protected:
	static DWORD	m_dwStatus;

};

#endif // !defined(AFX_CMONSTERSERVER_H__84170BF9_CB4B_4559_B900_790678A46F28__INCLUDED_)