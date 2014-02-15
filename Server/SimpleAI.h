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
 *	@file SimpleAI.h
 */

#if !defined(AFX_SIMPLEAI_H__33AB5540_9AF8_405E_9423_B2EC11F51808__INCLUDED_)
#define AFX_SIMPLEAI_H__33AB5540_9AF8_405E_9423_B2EC11F51808__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <time.h>
#include "Shared.h"
#include "WorldManager.h"
#include "cMonsterServer.h"

class SimpleAI  
{
public:
	SimpleAI();
	virtual ~SimpleAI();

	static bool		AddMonster			( DWORD dwGUID );
	static bool		AddPet				( DWORD dwGUID, DWORD dwOGUID );
	static bool		RemoveMonster		( DWORD dwGUID );
	static void     ExecuteActions		( );

	static void		SetAction			( DWORD dwGUID, DWORD dwEvent, WORD wDelay );

	static void		SetMoving			( DWORD dwGUID );
	static void		SetMovingComplete	( DWORD dwGUID );

	static void		SetUnderAttack		( DWORD dwGUID, DWORD dwTarget );
	static void		SetAttackComplete	( DWORD dwGUID );
	static void		SetTargetKilled		( DWORD dwTarget );
	static void		SetAttackEvent		( DWORD dwGUID, DWORD dwTarget, DWORD dwEvent, WORD wDelay );
	static void		MoveMonsters		( );

	static DWORD	Monsters[MAX_MONSTERS][8];

	static time_t	longtime;
	static time_t	NextActionTime;
};

#endif // !defined(AFX_SIMPLEAI_H__33AB5540_9AF8_405E_9423_B2EC11F51808__INCLUDED_)