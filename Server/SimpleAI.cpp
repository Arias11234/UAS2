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
 *	@file SimpleAI.cpp
 *	Implements functionality for monster AI.
 */

#include "SimpleAI.h"
#include "MasterServer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

DWORD	SimpleAI::Monsters[MAX_MONSTERS][8];

time_t  SimpleAI::NextActionTime;
time_t  SimpleAI::longtime;

/***************
 *	constructors/destructors
 **************/

SimpleAI::SimpleAI()
{
	for(DWORD i = 0; i < MAX_MONSTERS; i++)
	{
		Monsters[i][0] = 0; // GUID
		Monsters[i][1] = 0; // Next Action Time
		Monsters[i][2] = 0; // Action
		Monsters[i][3] = 0; // Target GUID
		Monsters[i][4] = 0; // Engaged in Combat
		Monsters[i][5] = 0; // Is Moving
		Monsters[i][6] = 0;
		Monsters[i][7] = 0;
	}
	time(&NextActionTime + ACTION_DELAY);
}

SimpleAI::~SimpleAI()
{

}

void SimpleAI::MoveMonsters( )
{
	for (int i = 0; i < MAX_MONSTERS; i++)
	{
		if ( Monsters[i][5] == 1)
		{
			cMonster* pcMonster = (cMonster *) cWorldManager::FindObject( Monsters[i][0] );
			cClient* pcClient = cClient::FindClient( pcMonster->m_dwTargetGUID );
			if ( pcMonster->m_fHasTarget && pcClient)
			{
				pcMonster->m_TargetLocation = pcClient->m_pcAvatar->m_Location;
			}
			if (cPhysics::Get3DRange( pcMonster->m_Location, pcMonster->m_TargetLocation ) > 0.07 || !pcMonster->m_fHasTarget )
			{
				cVelocity tarVel = cPhysics::GetTargetVelocity( pcMonster->m_Location, pcMonster->m_TargetLocation );
				cLocation newLoc = cPhysics::VelocityMove(pcMonster->m_Location, tarVel, 0.5f);

				if ( HIWORD(newLoc.m_dwLandBlock) != HIWORD(pcMonster->m_Location.m_dwLandBlock) )
				{
					cWorldManager::MoveRemObject( pcMonster );
					pcMonster->m_Location = newLoc;
					cWorldManager::MoveAddObject( pcMonster );
				}
				else
					pcMonster->m_Location = newLoc;

				pcMonster->m_Location.m_flZ = cPhysics::GetLandZ( newLoc );

				if ( cPhysics::Get3DRange( pcMonster->m_Location, pcMonster->m_TargetLocation ) < 0.01 )
				{
					SimpleAI::SetMovingComplete( pcMonster->m_dwGUID );
					pcMonster->m_fHasTarget = false;
					cMessage cmPos = pcMonster->SetPosition();
					cWorldManager::SendToAllInFocus( pcMonster->m_Location, cmPos, 3 );
					pcMonster->m_iPosUpdateCount = 0;
				}
				
				pcMonster->m_iPosUpdateCount++;
				if (pcMonster->m_iPosUpdateCount >= 4)
				{
					cMessage cmPos = pcMonster->SetPosition();
					cWorldManager::SendToAllInFocus( pcMonster->m_Location, cmPos, 3 );
					pcMonster->m_iPosUpdateCount = 0;
				}
			}
		}
	}
}

bool SimpleAI::AddMonster(DWORD dwGUID)
{
	bool fAdded = false;

	for(int i = 0; i < MAX_MONSTERS; i++)
	{
		if(Monsters[i][0] == 0)
		{
			time(&longtime);

			Monsters[i][0] = dwGUID;
			Monsters[i][1] = longtime + ACTION_DELAY;
			i = MAX_MONSTERS + 1;
			fAdded = true;
		}
	}
	return fAdded;
}

bool SimpleAI::RemoveMonster(DWORD dwGUID)
{
	bool fRemoved = false;

	for(int i = 0; i < MAX_MONSTERS; i++)
	{
		if(Monsters[i][0] == dwGUID)
		{
			Monsters[i][0] = 0;
			Monsters[i][1] = 0;
			Monsters[i][2] = 0;
			Monsters[i][3] = 0;
			Monsters[i][4] = 0;
			Monsters[i][5] = 0;
			Monsters[i][6] = 0;
			Monsters[i][7] = 0;

			i = MAX_MONSTERS + 1;
			fRemoved = true;
		}
	}
	return fRemoved;
}

void SimpleAI::ExecuteActions( )
{
	for(int i = 0; i < MAX_MONSTERS; i++)
	{
		time(&longtime);

		if((Monsters[i][0] != 0)&&(longtime > Monsters[i][1] ))
		{
			srand( timeGetTime( ) );

			int iRand = rand( );
			iRand = (iRand > 6400) ? iRand % 6400 : iRand;
			iRand = (iRand < 1600) ? 1600 + iRand : iRand;
			
			int delay = iRand/100; 
			Monsters[i][1] = longtime + delay;
			
			cObject* pcObject = cWorldManager::FindObject( Monsters[i][0] );
			if(pcObject)
			{
				cMonsterServer::ProcessAction(Monsters[i][0],Monsters[i][2], Monsters[i][4], Monsters[i][3]);
			}
			else
			{
				//Nothing to do
			}
		}
	}
}


void SimpleAI::SetAction( DWORD dwGUID, DWORD dwEvent, WORD wDelay )
{
	for(int i = 0; i < MAX_MONSTERS; i++)
	{
		if(Monsters[i][0] == dwGUID)
		{
			Monsters[i][1] = longtime + wDelay;
			Monsters[i][2] = dwEvent;
			Monsters[i][3] = 0;

			i = MAX_MONSTERS + 1;
		}
	}
}

void SimpleAI::SetMoving( DWORD dwGUID )
{
	for (int i=0; i < MAX_MONSTERS; i++)
	{
		if ( Monsters[i][0] == dwGUID )
		{
			Monsters[i][5] = 1;
			i = MAX_MONSTERS + 1;
		}
	}
}

void SimpleAI::SetMovingComplete( DWORD dwGUID )
{
	for (int i=0; i < MAX_MONSTERS; i++)
	{
		if ( Monsters[i][0] == dwGUID )
		{
			Monsters[i][5] = 0;
			i = MAX_MONSTERS + 1;
		}
	}
}

void SimpleAI::SetUnderAttack( DWORD dwGUID, DWORD dwTarget )
{
	for(int i = 0; i < MAX_MONSTERS; i++)
	{
		if(Monsters[i][0] == dwGUID)
		{
			if(Monsters[i][4] == 1)
			{
				Monsters[i][1] = longtime + 0;
				Monsters[i][2] = 5;
				//Monsters[i][3] = dwTarget;
				Monsters[i][4] = 1; // Engaged in Combat
			}
			else
			{
				Monsters[i][1] = longtime + 0;
				Monsters[i][2] = 1;
				Monsters[i][3] = dwTarget;
				Monsters[i][4] = 1; // Engaged in Combat
			}

			i = MAX_MONSTERS + 1;
		}
	}
}

void SimpleAI::SetAttackComplete( DWORD dwGUID )
{
	for(int i = 0; i < MAX_MONSTERS; i++)
	{
		if(Monsters[i][0] == dwGUID)
		{
			Monsters[i][1] = longtime + 1;
			Monsters[i][2] = 7;
			Monsters[i][3] = 0;
			Monsters[i][4] = 0; // Dis-Engaged from Combat

			i = MAX_MONSTERS + 1;
		}
	}
}

void SimpleAI::SetAttackEvent( DWORD dwGUID, DWORD dwTarget, DWORD dwEvent, WORD wDelay )
{
	for(int i = 0; i < MAX_MONSTERS; i++)
	{
		if(Monsters[i][0] == dwGUID)
		{
			Monsters[i][1] = longtime + wDelay;
			Monsters[i][2] = dwEvent;
			Monsters[i][3] = dwTarget;

			i = MAX_MONSTERS + 1;
		}
	}
}

void SimpleAI::SetTargetKilled( DWORD dwTarget )
{
	for(int i = 0; i < MAX_MONSTERS; i++)
	{
		if(Monsters[i][3] == dwTarget)
		{
			Monsters[i][1] = 5;
			Monsters[i][2] = 0;
			Monsters[i][3] = 0;
			Monsters[i][4] = 0; // Disengaged from combat
		}
	}
}