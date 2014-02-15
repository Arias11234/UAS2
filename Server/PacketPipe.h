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
 *	@file PacketPipe.h
 */

#ifndef __PACKETPIPE_H
#define __PACKETPIPE_H

#include <list>

#include "Message.h"
#include "CRC_Module.h"


typedef struct cFragment {
	BYTE		m_pbData[MAX_PACKET_SIZE];
	WORD		m_wSize;
	DWORD		m_dwSeed;
	
	cFragment( BYTE *pbData, WORD wSize )
		:	m_wSize( wSize + 36 )
	{
		CopyMemory( &m_pbData[36], pbData, wSize );
	}

	cFragment( )
		:	m_wSize( 0 )
	{
	}

	~cFragment( ) {}

} cFragment;

typedef struct cIgnoreRequest
{
	DWORD	m_dwZero;	//Sequence - Numeric identifier for the order in which this packet should be processed.
	DWORD	m_dwEight;	//Flags - Bitmask identifying what data follows the header.
	DWORD	m_dwCRC;	// CRC - Cyclic redundancy checksum verifying all data is authentic and undamaged.
	DWORD	m_dwNum;	// ?
	WORD	m_wUnk1; 	// Server Logical ID - Recipient identifier (index < 400) distinguishing the connection host.
	WORD	m_wUnk2;	// Time - Half-seconds since connection establishment that you received a packet.
	WORD	m_wSize;	// Size - Number of bytes following the header.
	WORD	m_wTable;	// Table - Unknown. Might involve CRC. Should match for sent/received headers.

	cIgnoreRequest( )
	{
		m_dwZero	= 0;
		m_dwEight	= 8;
		m_wSize		= 8;
		m_wTable	= 2;
	}
} cIgnoreRequest;

template< class _Type >
class cPacketPipe : public _Type
{
	friend class cClient;
	friend class cMasterServer;

public:
	cPacketPipe	( )	{}
	~cPacketPipe( ) 
	{
		while ( !m_lstQueuedPackets.empty( ) )
		{
			SAFEDELETE( m_lstQueuedPackets.front( ) )
			m_lstQueuedPackets.pop_front( );
		}

		while ( !m_lstSentPackets.empty( ) )
		{
			SAFEDELETE( m_lstSentPackets.front( ) )
			m_lstSentPackets.pop_front( );
		}
	}

	/**
	 *	Set the initial values.
	 */
	inline void Initialize( )
	{
		m_dwSendSequence	= 1;
		m_dwRecvSequence	= 1;
		m_dwMsgID			= 1;
		m_wTime				= 12;
		m_pdwSendCRC		= m_lpdwSendCRC;
		m_pdwRecvCRC		= m_lpdwRecvCRC;

		ResetCRCs( );
	}
	/**
	 *	Reset the values.
	 */
	inline void Reset( )
	{
		m_dwSendSequence	= 1;
		m_dwRecvSequence	= 1;
		m_dwMsgID			= 1;
		m_wTime				= 12;

		ResetCRCs( );
	}

	inline void SetSendSequence( DWORD dwSendSequence )
	{
		m_dwSendSequence = dwSendSequence;
	}

	/**
	 *	Creates a general CRC.
	 *
	 *	@param *pbSendBuffer - 
	 *	@param iSize - 
	 *
	 *	@return (dwCrc1 + dwCrc2) - 
	 */
	static inline DWORD CalcCRC( BYTE *pbSendBuffer, int iSize )
	{
		DWORD dwCrc1, dwCrc2, *pdwCrc;

		pdwCrc	= (DWORD *)&pbSendBuffer[8];
		*pdwCrc = 0xBADD70DD;
		dwCrc1	= GetMagicNumber( &pbSendBuffer[0x00], 0x14, TRUE );
		dwCrc2	= GetMagicNumber( &pbSendBuffer[0x14], iSize, FALSE );
		*pdwCrc = dwCrc1 + dwCrc2;

		return (dwCrc1 + dwCrc2);
	}

private:
	/**
	 *	Creates the fragment headers and adds the packet to the client's send queue.
	 *
	 *	@param *pbData - A pointer to the data to be sent.
	 *	@param wSize - The size of the data.
	 *	@param wGroup - The group to which the data belongs (used by AddFragmentHeader).
	 */
	void AddPacket( BYTE *pbData, WORD wSize, WORD wGroup )
	{
		if ( !wSize || !pbData )
			return ;

		WORD			wDataSize;
		cFragment		*pcFragment;

		if ( FindSpaceInQueue( pbData, wSize, wGroup ) )
			return ;

		WORD wSizeLeft		= wSize;
		WORD wFragmentCount	= wSize / MAX_DATA_SIZE;
		WORD wFragmentIndex	= 0;
		BYTE *pbCurData		= pbData;

		if ( wSize % MAX_DATA_SIZE )
			++wFragmentCount;

		wDataSize	= wSizeLeft > MAX_DATA_SIZE ? MAX_DATA_SIZE : wSizeLeft;
		pcFragment	= new cFragment( pbCurData, wDataSize );

		for ( ; wFragmentIndex < wFragmentCount; ++wFragmentIndex )
		{
			AddFragmentHeader( pcFragment->m_pbData + sizeof( cTransportHeader ), wDataSize, wFragmentIndex, wFragmentCount, wGroup );

			pbCurData	= pbCurData + wDataSize;
			wSizeLeft	-= wDataSize;
			wDataSize	= wSizeLeft > MAX_DATA_SIZE ? MAX_DATA_SIZE : wSizeLeft;

			m_lstQueuedPackets.push_back( pcFragment );

			if ( wDataSize )
				pcFragment = new cFragment( pbCurData, wDataSize );
		}

		++m_dwMsgID;
			
	}
	/**
	 *	Constructs the transport headers and sends the packets.
	 *
	 *	@param saSockAddr - The client's SOCKADDR_IN struct value.
	 */
	void SendQueuedPackets( SOCKADDR_IN& saSockAddr )
	{
		cTransportHeader *pcTH;
		cFragment *pcFragment;
		
		while ( !m_lstQueuedPackets.empty( ) )
		{
			pcFragment = m_lstQueuedPackets.front( );

			pcTH = reinterpret_cast< cTransportHeader * >( pcFragment->m_pbData );

			pcTH->m_dwSequence	= ++m_dwSendSequence;
			pcTH->m_dwFlags		= 0x0200;
			pcTH->m_wLogicalID	= m_wLogicalID;
			pcTH->m_wTime		= m_wTime;
			pcTH->m_wTotalSize	= pcFragment->m_wSize - sizeof( cTransportHeader );
			pcTH->m_wTable		= 0x0000;

			DWORD dwNewCRC, dwXorVal;
			//dwNewCRC = Calc200_CRC( pcFragment->m_pbData );
//
			dwNewCRC = GetMagicNumber(pcFragment->m_pbData + sizeof(cTransportHeader),pcTH->m_wTotalSize,1);
//
			dwXorVal = GetSendXORVal( m_pdwSendCRC );
			dwNewCRC ^= dwXorVal;
//			
			pcTH->m_dwCRC = 0xBADD70DD;
//
			//dwNewCRC += CalcTransportCRC( (DWORD *)pcFragment->m_pbData );

			dwNewCRC += GetMagicNumber(pcFragment->m_pbData, sizeof(cTransportHeader),1);
//
			pcTH->m_dwCRC = dwNewCRC;

			sendto( m_Socket, (char *)pcFragment->m_pbData, pcFragment->m_wSize, NULL, (sockaddr *)&saSockAddr, sizeof( SOCKADDR ) );

			pcFragment->m_dwSeed = dwXorVal;
			
			m_lstSentPackets.push_back( pcFragment );
			m_lstQueuedPackets.pop_front( );
		}
	}
	/**
	 *	Calculates the CRC for 0x0200 packets.
	 *
	 *	@param *pbPacket - 
	 *
	 *	@return dwCrc - 
	 */
	static inline DWORD Calc200_CRC( BYTE *pbPacket )
	{
		BYTE *pbPacketEnd = pbPacket + reinterpret_cast< cTransportHeader * > ( pbPacket )->m_wTotalSize + sizeof ( cTransportHeader );
		DWORD dwCrc = 0;

		for ( BYTE *pbFragment = pbPacket + sizeof ( cTransportHeader ); pbFragment < pbPacketEnd; )
		{
			WORD wLength = reinterpret_cast< cFragmentHeader * > ( pbFragment )->m_wFragmentLength;

			dwCrc += GetMagicNumber ( pbFragment, sizeof ( cFragmentHeader ), 1 ) + GetMagicNumber ( pbFragment + sizeof ( cFragmentHeader ), wLength - sizeof ( cFragmentHeader ), TRUE );
			pbFragment += wLength;
		}

		return dwCrc;
	}
	/**
	 *	Calculates the CRC for the transport header.
	 *
	 *	@param *pdwWoot - 
	 *
	 *	@return dwCrc - 
	 */
	static inline DWORD CalcTransportCRC( DWORD *pdwWoot )
	{
		DWORD dwOrg = pdwWoot[2];
		DWORD dwCrc	= 0;

		pdwWoot[2]	= 0xBADD70DD;
		dwCrc		+= GetMagicNumber( (BYTE *)pdwWoot, 20, TRUE );
		pdwWoot[2]	= dwOrg;

		return dwCrc;
	}
	/**
	 *	Seeds the CRC.
	 *
	 *	@param *pbBuf - A pointer to the data to use.
	 *	@param wSize - The size of the data.
	 *	@param fIncludeSize - 
	 *
	 *	@return dwCS - 
	 */
	static DWORD GetMagicNumber( BYTE *pbBuf, WORD wSize, BOOL fIncludeSize )
	{
		DWORD dwCS = 0;
		int i;

		if ( fIncludeSize )
			dwCS += wSize << 16;
		
		// sum up the DWORDs:
		for (i = 0; i < (wSize >> 2); ++i )
			dwCS += ((DWORD *)pbBuf)[i];
		
		// now any remaining bytes are summed in reverse endian
		int iShift = 3;
		for ( i = (i << 2); i < wSize; ++i )
		{
			dwCS += pbBuf[i] << (iShift * 8);
			--iShift;
		}

		return dwCS;
	}
	/**
	 *	Returns a ping packet.
	 *
	 *	@param saSockAddr - The client's SOCKADDR_IN struct value.
	 */
	void ReturnPing( SOCKADDR_IN& saSockAddr )
	{
		char szSendBuffer[0x40];

		DWORD *last = (DWORD *)&szSendBuffer[0x14];

		cTransportHeader *pcTH;
		pcTH = reinterpret_cast< cTransportHeader * >( szSendBuffer );
		pcTH->m_dwSequence	= m_dwSendSequence;
		pcTH->m_dwFlags		= 0x00000004;
		pcTH->m_wLogicalID	= m_wLogicalID;
		pcTH->m_wTime		= m_wTime;
		pcTH->m_wTotalSize	= 4;
		pcTH->m_wTable		= 0;

		*last = m_dwRecvSequence;

		CalcCRC( (BYTE *)szSendBuffer, 4 );
		sendto( m_Socket, (char *)szSendBuffer, 24, NULL, (SOCKADDR *)&saSockAddr, sizeof( SOCKADDR ) );
	}
	/**
	 *	Adds a packet to the queue.
	 *
	 *	@param cmPacket - The address of the packet message to add.
	 *	@param wGroup - The group to which the packet belongs (used by AddFragmentHeader).
	 */
	inline void AddPacket( cMessage& cmPacket, WORD wGroup )
	{
		WORD wSize = cmPacket.getSize( );

		BYTE *pbData = new BYTE[wSize];
		cmPacket.getData( 0, wSize, pbData );

		AddPacket( pbData, wSize, wGroup );

		SAFEDELETE_ARRAY( pbData )
	}
	/**
	 *	Checks to see if this packet can be crammed into another fragment.
	 *
	 *	@param *pbData - A pointer to the data.
	 *	@param wSize - The size of the data.
	 *	@param wGroup - The group to which the data belongs (used by AddFragmentHeader).
	 */
	BOOL FindSpaceInQueue( BYTE *pbData, WORD wSize, WORD wGroup )
	{	
		cFragment *pcFragment;

		std::list< cFragment * >::iterator itFragment = m_lstQueuedPackets.begin( );
		for ( ; itFragment != m_lstQueuedPackets.end( ); ++itFragment )
		{
			pcFragment = *itFragment;

			if ( pcFragment->m_wSize + wSize + sizeof( cFragmentHeader ) <= MAX_PACKET_SIZE )
			{
				CopyMemory( &pcFragment->m_pbData[pcFragment->m_wSize + sizeof( cFragmentHeader )], pbData, wSize );
				AddFragmentHeader( &pcFragment->m_pbData[pcFragment->m_wSize], wSize, 0, 1, wGroup );
				pcFragment->m_wSize += wSize + sizeof( cFragmentHeader );
				return TRUE;
			}
		}
		return FALSE;
	}
	/**
	 *	Creates the fragment header.
	 *
	 *	@param *pbStart - 
	 *	@param wSize - 
	 *	@param wFragmentIndex - 
	 *	@param wFragmentCount - 
	 *	@param wGroup - 
	 */
	inline void AddFragmentHeader( BYTE *pbStart, WORD wSize, WORD wFragmentIndex, WORD wFragmentCount, WORD wGroup )
	{	
		cFragmentHeader *pcFH = reinterpret_cast< cFragmentHeader * >( pbStart );

		pcFH->m_dwSequence		= m_dwMsgID;
		pcFH->m_dwObjectID		= 0x80000000;
		pcFH->m_wFragmentIndex	= wFragmentIndex;
		pcFH->m_wFragmentLength	= wSize + sizeof( cFragmentHeader );
		pcFH->m_wFragmentCount	= wFragmentCount;
		pcFH->m_wFragmentGroup	= wGroup;
	}
	/**
	 *	Resends packets that the client needs.
	 *
	 *	@param dwNumLostPackets - The number of packets lost.
	 *	@param *pdwMissingPackets - An array of the lost packets (of count dwNumLostPackets).
	 *	@param saSockAddr - The client's SOCKADDR_IN struct value.
	 */
	void ProcessLostPackets( DWORD dwNumLostPackets, DWORD *pdwMissingPackets, SOCKADDR_IN& saSockAddr )
	{

		DWORD dwNumIgnored = 0;

		std::vector< DWORD > vIgnoreList;
		vIgnoreList.reserve( dwNumLostPackets );

		cFragment *pcFragment;

		for ( DWORD dwCurrentPacket = 0; dwCurrentPacket < dwNumLostPackets; ++dwCurrentPacket )
		{
			std::list< cFragment * >::iterator itFragment = m_lstSentPackets.begin( );
			for ( ; itFragment != m_lstSentPackets.end( ); ++itFragment )
			{
				pcFragment = *itFragment;
				if ( *((DWORD *)pcFragment->m_pbData) == pdwMissingPackets[dwCurrentPacket] )
					break;
			}

			if ( itFragment != m_lstSentPackets.end( ) )
			{
				cTransportHeader *pcTH = reinterpret_cast< cTransportHeader * >( pcFragment->m_pbData );

				pcTH->m_dwFlags = 0x00000201;
				pcTH->m_wTime	= m_wTime;

				DWORD dwNewCRC;
				dwNewCRC = Calc200_CRC( pcFragment->m_pbData );
				dwNewCRC ^= pcFragment->m_dwSeed;
				dwNewCRC += CalcTransportCRC( (DWORD *)pcFragment->m_pbData );

				pcTH->m_dwCRC = dwNewCRC;

				sendto( m_Socket, (char *)pcFragment->m_pbData, pcFragment->m_wSize, NULL, (SOCKADDR *)&saSockAddr, sizeof( SOCKADDR ) );
			}
			else 
			{
				vIgnoreList.push_back( pdwMissingPackets[dwCurrentPacket] );
				++dwNumIgnored;
			}
		}

		if ( !vIgnoreList.empty( ) )
		{
			BYTE pbSendBuffer[0x1E4];

			cIgnoreRequest irOut;
			irOut.m_dwNum	= dwNumIgnored;
			irOut.m_wUnk1	= m_wLogicalID;
			irOut.m_wUnk2	= m_wTime;
			irOut.m_wSize	= 4 + dwNumIgnored * 4;

			CopyMemory( pbSendBuffer, &irOut, sizeof( cIgnoreRequest ) );

			std::vector< DWORD >::iterator itIgnore = vIgnoreList.begin( );
			for ( DWORD dwCount = 0; itIgnore != vIgnoreList.end( ); ++itIgnore, ++dwCount )
				CopyMemory( pbSendBuffer + sizeof( irOut ) + dwCount * 4, &itIgnore, sizeof( DWORD ) );

			irOut.m_dwCRC = CalcCRC( pbSendBuffer, 4 + dwNumIgnored * 4 );

			sendto( m_Socket, (char*)pbSendBuffer, sizeof( irOut ) + dwNumIgnored * 4, NULL, (SOCKADDR *)&saSockAddr, sizeof( SOCKADDR ) );

			vIgnoreList.clear( );
		}
		
	}
	/**
	 *	Removes packets from the m_lstSentPackets list.
	 *
	 *	@param dwLastGoodSeq - 
	 */
	inline void ClearSentPackets( DWORD dwLastGoodSeq )
	{
		cFragment *pcFragment;

		while ( !m_lstSentPackets.empty( ) )
		{
			pcFragment = m_lstSentPackets.front( );

			if ( *((DWORD *)pcFragment->m_pbData) > dwLastGoodSeq )
				break;
			else
			{
				m_lstSentPackets.pop_front( );
				SAFEDELETE( pcFragment )
			}
		}
	}
	inline void ResetCRCs( )
	{
		GenerateCRCs( m_dwSendCRCSeed, m_dwRecvCRCSeed, m_pdwSendCRC, m_pdwRecvCRC );
	}

	DWORD		m_lpdwSendCRC[3];
	DWORD		m_lpdwRecvCRC[3];
	DWORD		*m_pdwSendCRC;
	DWORD		*m_pdwRecvCRC;
	DWORD		m_dwSendSequence;
	DWORD		m_dwRecvSequence;
	DWORD		m_dwMsgID;
	WORD		m_wTime;
	//SOCKADDR_IN m_saSockAddr;

	std::list< cFragment * > m_lstQueuedPackets;
	std::list< cFragment * > m_lstSentPackets;
};

#endif	// #ifndef __PACKETPIPE_H