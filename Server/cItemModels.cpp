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
 *	@file cItemModels.cpp
 *	Implements functionality for item models.
 *
 *	An item model comprises the generic traits shared among a type of object with that model.
 */

#include "cItemModels.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ITEM_MODEL_TABLE_SIZE 4000

//Cube: Reduce memory load here.  We don't need this much allocated.
cItemModels *cItemModels::m_lpcHashTable[ITEM_MODEL_TABLE_SIZE];

/**
 *	Erases all item models from the item model hash list.
 */
void cItemModels::Hash_Erase()
{
	cItemModels *pcItemModels, *pcPrevModel;
	for ( int i = 0; i < ITEM_MODEL_TABLE_SIZE; ++i )
	{
		pcItemModels = m_lpcHashTable[i];
		while ( pcItemModels )
		{
			pcPrevModel	= pcItemModels;
			pcItemModels	= pcItemModels->m_pcNext;

			Hash_Remove( pcPrevModel );
		}
	}
}

/**
 *	Removes an item model from the item model hash list.
 *
 *	@param *pcModel - A pointer to the item model to be removed.
 */
void cItemModels::Hash_Remove( cItemModels *pcModel )
{
	SAFEDELETE( pcModel )
}

/**
 *	Finds an item model.
 *
 *	This function is called when a particular item model needs to be found.
 *	The search is performed by searching for the item model's model ID.
 *
 *	@param dwModelID - The item model's model ID.
 *
 *	@return *cItemModels - A pointer to the item model.
 */
cItemModels *cItemModels::FindModel( DWORD dwModelID )
{
	cItemModels *pcModel;
	for ( int i = 0; i < ITEM_MODEL_TABLE_SIZE; ++i )
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