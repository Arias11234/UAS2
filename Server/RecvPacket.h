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
 *	@file RecvPacket.h
 */

#ifndef __RECVPACKET_H
#define __RECVPACKET_H

#include <cassert>

#include "Shared.h"

class cRecvPacket
{
public:
	cRecvPacket( ) 
		:	m_wSize( 0 )
	{
		m_pcTH = reinterpret_cast< cTransportHeader * >( m_bData );
	}

	~cRecvPacket( ) {}

	inline DWORD	GetSequence	( )	{ return m_pcTH->m_dwSequence;	}
	inline DWORD	GetFlags	( )	{ return m_pcTH->m_dwFlags;		}
	inline DWORD	GetCRC		( ) { return m_pcTH->m_dwCRC;		}
	inline WORD		GetLogicalID( )	{ return m_pcTH->m_wLogicalID;	}
	inline WORD		GetTime		( )	{ return m_pcTH->m_wTime;		}
	inline WORD		GetTotalSize( )	{ return m_pcTH->m_wTotalSize;	}
	inline WORD		GetTable	( ) { return m_pcTH->m_wTable;		}

	inline BYTE	*GetPayloadFront( )					{ return m_bData + sizeof( cTransportHeader );				}
	inline BYTE	*GetPayload		( WORD wOffset )	{ return m_bData + sizeof( cTransportHeader ) + wOffset;	}
	inline BYTE	*GetPayloadBack	( )					{ return m_bData + sizeof( cTransportHeader ) + m_pcTH->m_wTotalSize;	}
	
	inline void CopyPayload( WORD wOffset, void *pDest, WORD wSize )
	{ 
		CopyMemory( pDest, m_bData + sizeof( cTransportHeader ) + wOffset, wSize ); 
	}

	inline BYTE& operator[]( int n )
	{
	#ifdef _DEBUG
		WORD wMaxSize = GetTotalSize( ) + sizeof( cTransportHeader );
		if ( wMaxSize > 0 )
			assert( n < wMaxSize );
	#endif

		return m_bData[n];
	}

	inline int GetPadding( BYTE *pbStart );

	BYTE		m_bData[MAX_PACKET_SIZE];
	WORD		m_wSize;
	SOCKADDR_IN m_saSockAddr;

private:
	cTransportHeader *m_pcTH;
};

inline int cRecvPacket::GetPadding( BYTE *pbStart )
{
	WORD wSize = *(WORD *)pbStart;
	pbStart += (sizeof( WORD ) + wSize);

	DWORD dwDiff = pbStart - m_bData;

	if( dwDiff < 4 )
		return 4 - dwDiff; 
	else if ( (dwDiff % 4) != 0 ) 
		return 4 - (dwDiff % 4); 
	else 
		return 0; 
}

#endif	// #ifndef __RECVPACKET_H