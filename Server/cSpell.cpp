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
 *	@file cSpell.cpp
 *	Implements the cSpells class.
 */

#include "cSpell.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

cSpell *cSpell::m_lpcHashTable[3000];
cSpellComp *cSpellComp::m_lpcHashTable[200];

/**
 *	Erases all spells from the spell hash list.
 */
void cSpell::Hash_Erase()
{
	cSpell *pcSpell, *pcPrevSpell;
	for ( int i = 0; i < 3000; ++i )
	{
		pcSpell = m_lpcHashTable[i];
		while ( pcSpell )
		{
			pcPrevSpell	= pcSpell;
			pcSpell		= pcSpell->m_pcNext;

			Hash_Remove( pcPrevSpell );
		}
	}
}

/**
 *	Removes a spell from the spell hash list.
 *
 *	@param *pcSpell - A pointer to the spell to be removed.
 */
void cSpell::Hash_Remove( cSpell *pcSpell )
{
	SAFEDELETE( pcSpell )
}

/**
 *	Finds a spell.
 *
 *	This function is called when a particular spell needs to be found.
 *	The search is performed by searching for the spell's spell ID.
 *
 *	@param dwSpellID - The spell's spell ID.
 *
 *	@return *cSpell - A pointer to the spell.
 */
cSpell *cSpell::FindSpell( DWORD dwSpellID )
{
	cSpell *pcSpell;
	for ( int i = 0; i < 3000; ++i )
	{
		pcSpell = m_lpcHashTable[i];
		while ( pcSpell )
		{
			if ( pcSpell->m_dwSpellID )
			{
				if ( dwSpellID == pcSpell->m_dwSpellID )
					return pcSpell;
			}
			pcSpell = pcSpell->m_pcNext;
		}
	}
	return NULL;
}

BYTE cSpell::GetWindup( )
{
	cSpellComp *pcComp = cSpellComp::FindComp( m_dwLevel );
	if (!pcComp)
		return 0x70;
	return pcComp->m_dwAnimID & 0xFF;
}

int cSpell::GetWindupDelay( )
{
	cSpellComp *pcComp = cSpellComp::FindComp( m_dwLevel );
	if (!pcComp)
		return 10;
	return pcComp->m_flChargeID * 10 - pcComp->m_flChargeID + 1;
}

BYTE cSpell::GetCastAnim( )
{
	int iComp;

	if (m_dwComponents[5] == 0)
	{
		iComp = m_dwComponents[4] - m_dwComponents[0] + 1;
		cSpellComp *pcComp = cSpellComp::FindComp( iComp );
		return pcComp->m_dwAnimID & 0xFF;
	}
	else if (m_dwComponents[6] == 0)
	{
		iComp = m_dwComponents[5] - m_dwComponents[0] + 2;
		cSpellComp *pcComp = cSpellComp::FindComp( iComp );
		return pcComp->m_dwAnimID & 0xFF;
	}
	else if (m_dwComponents[7] == 0)
	{
		if (m_dwLevel != 0)
		{
			iComp = m_dwComponents[6] - m_dwComponents[0] + m_dwLevel;
			cSpellComp *pcComp = cSpellComp::FindComp( iComp );
			return pcComp->m_dwAnimID & 0xFF;
		}
	}
	else
	{
		if (m_dwLevel != 0)
		{
			iComp = m_dwComponents[7] - m_dwComponents[0] + m_dwLevel;
			cSpellComp *pcComp = cSpellComp::FindComp( iComp );
			return pcComp->m_dwAnimID & 0xFF;
		}
	}

	return 0x33;
}

int cSpell::GetCastAnimDelay( )
{
	int iComp;

	if (m_dwComponents[5] == 0)
	{
		iComp = m_dwComponents[4] - m_dwComponents[0] + 1;
		cSpellComp *pcComp = cSpellComp::FindComp( iComp );
		return pcComp->m_flChargeID * 10;
	}
	else if (m_dwComponents[6] == 0)
	{
		iComp = m_dwComponents[5] - m_dwComponents[0] + 2;
		cSpellComp *pcComp = cSpellComp::FindComp( iComp );
		return pcComp->m_flChargeID * 10;
	}
	else if (m_dwComponents[7] == 0)
	{
		if (m_dwLevel != 0)
		{
			iComp = m_dwComponents[6] - m_dwComponents[0] + m_dwLevel;
			cSpellComp *pcComp = cSpellComp::FindComp( iComp );
			return pcComp->m_flChargeID * 10;
		}
	}
	else
	{
		if (m_dwLevel != 0)
		{
			iComp = m_dwComponents[7] - m_dwComponents[0] + m_dwLevel;
			cSpellComp *pcComp = cSpellComp::FindComp( iComp );
			return pcComp->m_flChargeID * 10;
		}
	}

	return 20;
}

cMessage cSpell::GetCastWords( )
{
	cMessage cmSpellWords;
	char szWords[14];
	cSpellComp *pcComp1, *pcComp2, *pcComp3;
	int iComp;
	
	static BYTE bSpellWords[] = {
		0x37, 0x00, 0x00, 0x00, 
		0x0E, 0x00, 0x5A, 0x6F, 
		0x6A, 0x61, 0x6B, 0x20, 
		0x51, 0x75, 0x61, 0x66, 
		0x65, 0x74, 0x68, 0x00, 
	};

	if (m_dwLevel != 0)
	{
		if (m_dwComponents[5] == 0)
		{
			iComp = m_dwComponents[1] - m_dwComponents[0] + 1;
			pcComp1 = cSpellComp::FindComp( iComp );
			iComp = m_dwComponents[2] - m_dwComponents[0] + 1;
			pcComp2 = cSpellComp::FindComp( iComp );
			iComp = m_dwComponents[3] - m_dwComponents[0] + 1;
			pcComp3 = cSpellComp::FindComp( iComp );

		}
		else if (m_dwComponents[6] == 0)
		{
			iComp = m_dwComponents[2] - m_dwComponents[0] + 2;
			pcComp1 = cSpellComp::FindComp( iComp );
			iComp = m_dwComponents[3] - m_dwComponents[0] + 2;
			pcComp2 = cSpellComp::FindComp( iComp );
			iComp = m_dwComponents[4] - m_dwComponents[0] + 2;
			pcComp3 = cSpellComp::FindComp( iComp );
		}
		else
		{
			if (m_dwLevel != 0)
			{
				iComp = m_dwComponents[2] - m_dwComponents[0] + m_dwLevel;
				pcComp1 = cSpellComp::FindComp( iComp );
				iComp = m_dwComponents[4] - m_dwComponents[0] + m_dwLevel;
				pcComp2 = cSpellComp::FindComp( iComp );
				iComp = m_dwComponents[5] - m_dwComponents[0] + m_dwLevel;
				pcComp3 = cSpellComp::FindComp( iComp );
			}
		}
		wsprintf( szWords, "%s %s%s", pcComp1->m_strWords.c_str(), pcComp2->m_strWords.c_str(), pcComp3->m_strWords.c_str() );
		CopyMemory( &bSpellWords[6], &szWords, 14 );
	}

	cmSpellWords.pasteData(bSpellWords,20);

	return cmSpellWords;
}

void cSpellComp::Hash_Erase()
{
	cSpellComp *pcComp, *pcPrevComp;
	for ( int i = 0; i < 200; ++i )
	{
		pcComp = m_lpcHashTable[i];
		while ( pcComp )
		{
			pcPrevComp	= pcComp;
			pcComp		= pcComp->m_pcNext;

			Hash_Remove( pcPrevComp );
		}
	}
}

void cSpellComp::Hash_Remove( cSpellComp *pcComp )
{
	SAFEDELETE( pcComp )
}

cSpellComp *cSpellComp::FindComp( DWORD dwCompID )
{
	cSpellComp *pcComp;
	for ( int i = 0; i < 200; ++i )
	{
		pcComp = m_lpcHashTable[i];
		while ( pcComp )
		{
			if ( pcComp->m_dwCompID )
			{
				if ( dwCompID == pcComp->m_dwCompID )
					return pcComp;
			}
			pcComp = pcComp->m_pcNext;
		}
	}
	return NULL;
}