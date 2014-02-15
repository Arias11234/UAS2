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
 *	@file cWObjectModels.cpp
 *	Implements the cWObjectModels class.
 *
 *	Author: G70mb2
 */

#include "cWObjectModels.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define WORLD_OBJECT_MODEL_TABLE_SIZE 1000

cWObjectModels *cWObjectModels::m_lpcHashTable[WORLD_OBJECT_MODEL_TABLE_SIZE];

/**
 *	Erases all world object models from the world object model hash list.
 */
void cWObjectModels::Hash_Erase()
{
	cWObjectModels *pcModels, *pcPrevModel;
	for ( int i = 0; i < WORLD_OBJECT_MODEL_TABLE_SIZE; ++i )
	{
		pcModels = m_lpcHashTable[i];
		while ( pcModels )
		{
			pcPrevModel	= pcModels;
			pcModels	= pcModels->m_pcNext;

			Hash_Remove( pcPrevModel );
		}
	}
}

/**
 *	Removes a world object model from the world object model hash list.
 *
 *	@param *pcModel - A pointer to the world object model to be removed.
 */
void cWObjectModels::Hash_Remove( cWObjectModels *pcModel )
{
	SAFEDELETE( pcModel )
}

/**
 *	Finds a world object model.
 *
 *	This function is called when a particular world object model needs to be found.
 *	The search is performed by searching for the world object model's model ID.
 *
 *	@param dwModelID - The world object model's model ID.
 *
 *	@return *cWObjectModels - A pointer to the world object model.
 */
cWObjectModels *cWObjectModels::FindModel( DWORD dwModelID )
{
	cWObjectModels  *pcModel;
	for ( int i = 0; i < WORLD_OBJECT_MODEL_TABLE_SIZE; ++i )
	{
		pcModel = m_lpcHashTable[i];
		while ( pcModel )
		{
			if ( pcModel->m_dwModelID )
			{
				if ( dwModelID == pcModel->m_dwModelID )
					return pcModel;
			}
			pcModel = pcModel->m_pcNext;
		}
	}
	return NULL;
}