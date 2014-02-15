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
 *	@file CorpseCleaner.cpp
 *	Implements functionality for corpse addition and removal to and from the landscape.
 *
 *	The CorseCleaner object contains an array of all corpses presently on the landscape.
 *	This class is referenced whenever a corpse is created, removed, or check to be removed.
 */

#include "CorpseCleaner.h"

DWORD	CorpseCleaner::Corpses[MAX_CORPSES][4];
time_t  CorpseCleaner::NextCleanTime;
time_t  CorpseCleaner::longtime;

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of of the CorpseCleaner corpse array.
 *
 *	Called whenever the CorpseCleaner object should be initialized.
 */
CorpseCleaner::CorpseCleaner()
{
	for(DWORD i = 0; i < MAX_CORPSES; i++)
	{
		Corpses[i][0] = 0; // Objects GUID
		Corpses[i][1] = 0; // Decay Timer
		Corpses[i][2] = 0; // Is this a ReSpawnable object 
						   // 0 = No, #>0 is delay time for respawn after corpse is removed
		Corpses[i][3] = 0; // Unused
	}
	time(&NextCleanTime + DELAY);	
}

CorpseCleaner::~CorpseCleaner()
{

}

/**********
 *	methods
 *********/

/**
 *	Handles the addition of a corpse to the CorpseCleaner.
 *
 *	This function should be called whenever a corpse is created.
 *	@param dwGUID - The GUID of the corpse to add
 *	@param dwDelay - The default length of time for a corpse to remain on the landscape
 *	@param dwReSpawn - The additional delay for the the particular corpse
 */
bool CorpseCleaner::AddCorpse(DWORD dwGUID, DWORD dwDelay, DWORD dwReSpawn)
{
	bool fAdded = false;
	DWORD dwCleanupDelay;
	if(dwDelay == 0)
	{
		dwCleanupDelay = CORPSE_DELAY;
	}
	else
	{
		dwCleanupDelay = dwDelay;
	}

	for(int i = 0; i < MAX_CORPSES; i++)
	{
		if(Corpses[i][0] == 0)
		{
			time(&longtime);

			Corpses[i][0] = dwGUID;
			Corpses[i][1] = longtime + dwCleanupDelay;
			if(dwReSpawn == 0)
			{
				Corpses[i][2] = 0;
			}
			else
			{
				Corpses[i][2] = longtime + dwReSpawn + dwCleanupDelay;
			}
			i = MAX_CORPSES + 1;
			fAdded = true;
		}
	}
	return fAdded;
}

/**
 *	Handles the removal of a corpse from the CorpseCleaner.
 *
 *	This function should be called whenever a corpse is removed.
 *	@param dwGUID - The GUID of the corpse to remove
 */
bool CorpseCleaner::RemoveCorpse(DWORD dwGUID)
{
	bool fRemoved = false;

	for(int i = 0; i < MAX_CORPSES; i++)
	{
		if(Corpses[i][0] == dwGUID)
		{
			Corpses[i][0] = 0;
			Corpses[i][1] = 0;
			Corpses[i][2] = 0; 
			i = MAX_CORPSES + 1;
			fRemoved = true;
		}
	}
	return fRemoved;
}

/**
 *	Handles the removal of a corpse from the landscape.
 *
 *	This function should be called periodically to check for corpse removal.
 *	If a corpse is to be removed, the corpse is removed from the given landblock list and from the CorpseCleaner list
 */
void CorpseCleaner::Cleanup()
{
	for(int i = 0; i < MAX_CORPSES; i++)
	{
		time(&longtime);

		if( ( Corpses[i][0] != 0 ) && ( longtime > Corpses[i][1] ) && ( Corpses[i][1] != 0 ) )
		{
			cObject* pcObject = cWorldManager::FindObject( Corpses[i][0] );
			if(pcObject)
			{
				// Remove the Corpse from the landscape
				cMessage cmRemModel;
				cmRemModel << 0xF747L << Corpses[i][0] << DWORD( 1 );
				cWorldManager::SendToAllWithin( 5, pcObject->m_Location, cmRemModel, 3 );
				SimpleAI::RemoveMonster(Corpses[i][0]);

				if( Corpses[i][2] == 0 )
				{
					RemoveCorpse( Corpses[i][0] );
					
					// Destory the Object completely
					cWorldManager::RemoveObject( pcObject );
				}
				else  
				{
					// this is a Respawning Object So Do Not Destroy it
					// Set Corpse Timer to zero
					Corpses[i][1] = 0;
					
					// And Remove from the Landblock
					cWorldManager::MoveRemObject( pcObject );
					pcObject->m_Location.m_dwLandBlock = 0xE9E9002F;
					cWorldManager::MoveAddObject( pcObject );
				}
			}
			else
			{
				//Nothing to Do Here - Object is not valid
			}
		}
		else
		{
		}

		if( ( Corpses[i][0] != 0 ) && ( longtime > Corpses[i][2] ) && ( Corpses[i][1] == 0 ))
		{
			// ReSpawn Activation code
			cObject* pcObjectb = cWorldManager::FindObject( Corpses[i][0] );
			if(pcObjectb)
			{
				// ReSpawn the Monster
				pcObjectb->ReSpawn( pcObjectb );
				
				// Finally Clear the entry from the Corpse Cleaner array
				RemoveCorpse( Corpses[i][0] );
			}
			else
			{
				//Nothing to Do Here - Object is not valid
				RemoveCorpse( Corpses[i][0] );
			}

		}
		else
		{
			// Do absolutely nothing 
		}
	}
	// Set time of Next Cleanup Run
	time(&NextCleanTime + DELAY);
}