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
 *	@file CorpseCleaner.h
 */

#if !defined(AFX_ORPSECLEANER_H__DC16D656_DC96_492A_834C_14B0D779C272__INCLUDED_)
#define AFX_ORPSECLEANER_H__DC16D656_DC96_492A_834C_14B0D779C272__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <time.h>
#include "Shared.h"
#include "WorldManager.h"
#include "SimpleAI.h"


class CorpseCleaner  
{
public:
	CorpseCleaner();
	virtual ~CorpseCleaner();
	static void		Cleanup();
	static bool		AddCorpse(DWORD dwGUID, DWORD dwDelay = 0,DWORD dwReSpawn = 0);
	static bool		RemoveCorpse(DWORD dwGUID);
	static DWORD	Corpses[MAX_CORPSES][4];
	static time_t	longtime;
	static time_t	NextCleanTime;

};

#endif // !defined(AFX_ORPSECLEANER_H__DC16D656_DC96_492A_834C_14B0D779C272__INCLUDED_)