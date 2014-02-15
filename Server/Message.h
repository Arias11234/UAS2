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
 *	@file Message.h
 */

#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <deque>
#include <vector>
#include <cassert>

#include "Shared.h"

#define BUFFER_BLOCK_SIZE 1024

class cMessage
{
	class cBlock
	{
		friend class cMessage;

		mutable BYTE *m_pStart, *m_pUsed;
	
	public:
		
		cBlock ();
		cBlock ( const cBlock &from );
		~cBlock ();

		BOOL pasteData ( BYTE *&pData, int &nBytes );
		int getAlign ( int nBoundary ) const;
		BOOL pasteAlign ( int &nBytes );

		size_t getUsed () const
		{
			return ( m_pUsed - m_pStart );
		}

		BOOL getData ( size_t &nOffset, size_t &nLength, BYTE * &pBuffer ) const;

	private:
		// Disabled functions
		cBlock &operator= ( const cBlock & );
	};

	typedef std::deque< cBlock > cBlockList;
	cBlockList m_blocks;

	// Place a block of data into the message
public:
	cMessage ();
	cMessage (BYTE* pbPacket, int nSize);
	~cMessage( )
	{ 
		while ( !m_blocks.empty( ) )
			m_blocks.pop_front( );
	}

	// Basic functions for inserting data
	void pasteData ( BYTE *pData, int nBytes );
	void pasteAlign ( int nBoundary );
	void pasteString ( const char *str );

	// Functions for retrieving data
	size_t getSize () const;
	void getData ( size_t nOffset, size_t nLength, BYTE *pBuffer ) const;

	template< class DataT >
	inline void paste ( DataT dt )
	{
		pasteData ( reinterpret_cast< BYTE * > ( &dt ), sizeof ( DataT ) );
	}

	// String pastes
	template<>
	inline void paste ( char *str )
	{
		pasteString ( static_cast< const char * >( str ) ); 
	}

	template<>
	inline void paste ( const char *str )
	{
		pasteString ( static_cast< const char * >( str ) ); 
	}
 
	// Collection pastes
	template< class ItemT >
	void paste ( std::vector< ItemT > &vec )
	{
		for ( std::vector< ItemT >::iterator i = vec.begin(); i != vec.end(); ++ i )
			i->encode ( *this );

		// Always realign after doing a vector
		pasteAlign ( sizeof ( unsigned long ) );
	}

	void CannedData( BYTE *pbData, DWORD dwSize )
	{
		pasteData( pbData, dwSize );
	}

	inline BYTE& operator[]( int iElem ) const 
	{ 
//	#ifdef _DEBUG 
//		if ( getSize( ) > 0 ) 
//			assert( iElem < getSize( ) );
//	#endif
		return (m_blocks.begin( ) + (iElem / BUFFER_BLOCK_SIZE))->m_pStart[iElem % BUFFER_BLOCK_SIZE];
	}

private:
	class cBlockDispenser
	{
		HANDLE m_hHeap;

	public:
		cBlockDispenser ();
		~cBlockDispenser ();

		BYTE *get ();
		void release (BYTE *pBlock);
	};

	static cBlockDispenser g_dispenser;

	friend cBlock;
};

template< class DataT >
cMessage &operator<< ( cMessage &buf, DataT t )
{
   buf.paste ( t );
   return buf;
}

#endif	// #ifndef __MESSAGE_H