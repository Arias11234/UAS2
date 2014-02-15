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
 *	@file cClient.cpp
 *	Implements functionality for the cClient class.
 */

#include "Client.h"
#include "WorldManager.h"

cClient *cClient::m_lpcHashTable[1020];

/**
 *	Erases all clients from the client hash list.
 */
void cClient::Hash_Erase( )
{
	cClient *pcClient, *pcPrevClient;
	for ( int i = 0; i < 1020; ++i )
	{
		pcClient = m_lpcHashTable[i];
		while ( pcClient )
		{
			pcPrevClient	= pcClient;
			pcClient		= pcClient->m_pcNext;

			Hash_Remove( pcPrevClient );
		}
	}
}

/**
 *	Removes a client from the client hash list.
 *
 *	This function is called when a client disconnects from the server.
 *
 *	@param *pcClient - A pointer to the client to be removed.
 */
void cClient::Hash_Remove( cClient *pcClient )
{
	if ( pcClient->m_pcAvatar )
	{
		pcClient->m_pcAvatar->SaveToDB();
		cWorldManager::RemoveClient( pcClient );
	}

	SAFEDELETE( pcClient )
}

/**
 *	Finds a client.
 *
 *	This function is called when a particular client needs to be found.
 *	The search is performed by searching for the client's avatar's GUID.
 *
 *	@param dwGUID - The client's avatar's GUID.
 *
 *	@return *cClient - A pointer to the client.
 */
cClient *cClient::FindClient( DWORD dwGUID )
{
	cClient *pcClient;
	for ( int i = 0; i < 1020; ++i )
	{
		pcClient = m_lpcHashTable[i];
		while ( pcClient )
		{
			if ( pcClient->m_pcAvatar )
			{
				if ( dwGUID == pcClient->m_pcAvatar->GetGUID( ) )
					return pcClient;
			}
			pcClient = pcClient->m_pcNext;
		}
	}
	return NULL;
}

/**
 *	Finds a client.
 *
 *	This function is called when a particular client needs to be found.
 *	The search is performed by searching for the client's avatar's name.
 *
 *	@param szName - The client's avatar's name.
 *
 *	@return *cClient - A pointer to the client.
 */
cClient *cClient::FindClient( char *szName )
{
	cClient *pcClient;
	for ( int i = 0; i < 1020; ++i )
	{
		pcClient = m_lpcHashTable[i];
		while ( pcClient )
		{
			if ( pcClient->m_pcAvatar )
			{
				if ( !lstrcmpi( szName, pcClient->m_pcAvatar->TokenlessName( ) ) )
					return pcClient;
			}
			pcClient = pcClient->m_pcNext;
		}
	}
	return NULL;
}

/**
 *	Finds an avatar.
 *
 *	This function is called when a particular avatar needs to be found.
 *	The search is performed by searching for the client's avatar's GUID.
 *
 *	@param dwGUID - The client's avatar's GUID.
 *
 *	@return *cAvatar - A pointer to the avatar.
 */
cAvatar *cClient::FindAvatar( DWORD dwGUID )
{
	cClient *pcClient;
	for ( int i = 0; i < 1020; ++i )
	{
		pcClient = m_lpcHashTable[i];
		while ( pcClient )
		{
			if ( pcClient->m_pcAvatar )
			{
				if ( dwGUID == pcClient->m_pcAvatar->GetGUID( ) )
					return pcClient->m_pcAvatar;
			}
			pcClient = pcClient->m_pcNext;
		}
	}
	return NULL;
}

/**
 *	Finds an avatar.
 *
 *	This function is called when a particular avatar needs to be found.
 *	The search is performed by searching for the client's avatar's name.
 *
 *	@param szName - The client's avatar's name.
 *
 *	@return *cAvatar - A pointer to the avatar.
 */
cAvatar *cClient::FindAvatar( char *szName )
{
	cClient *pcClient;
	for ( int i = 0; i < 1020; ++i )
	{
		pcClient = m_lpcHashTable[i];
		while ( pcClient )
		{
			if ( pcClient->m_pcAvatar )
			{
				if ( !lstrcmpi( szName, pcClient->m_pcAvatar->TokenlessName( ) ) )
					return pcClient->m_pcAvatar;
			}
			pcClient = pcClient->m_pcNext;
		}
	}
	return NULL;
}