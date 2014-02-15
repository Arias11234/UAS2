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
 *	@file Message.cpp
 *	Encapsulates cMessage functionality.
 */

#include "Message.h"

#pragma intrinsic( memcpy, memset, strlen )
					
cMessage::cBlock::cBlock ()
: m_pStart ( g_dispenser.get () ),
m_pUsed ( m_pStart )
{
}

cMessage::cBlock::cBlock ( const cBlock &from )
: m_pStart ( from.m_pStart ),
m_pUsed ( from.m_pUsed )
{
   from.m_pStart = NULL;
   from.m_pUsed = NULL;
}

cMessage::cBlock::~cBlock ()
{
   if ( m_pStart != NULL )
      g_dispenser.release ( m_pStart );
}

BOOL cMessage::cBlock::pasteData ( BYTE * &pData, int &nBytes )
{
   int nAvailable = BUFFER_BLOCK_SIZE - ( m_pUsed - m_pStart );
   if ( nAvailable < nBytes )
   {
      // Paste what we can
      CopyMemory( m_pUsed, pData, nAvailable );
      nBytes -= nAvailable;
      m_pUsed += nAvailable;

      return FALSE;
   }

   CopyMemory( m_pUsed, pData, nBytes );
   m_pUsed += nBytes;

   return TRUE;
}

int cMessage::cBlock::getAlign ( int nBoundary ) const
{
   int nAlign = ( m_pUsed - m_pStart ) % nBoundary;
   return ( nAlign == 0 ) ? 0 : ( nBoundary - nAlign );
}

BOOL cMessage::cBlock::pasteAlign ( int &nBytes )
{
   int nAvailable = BUFFER_BLOCK_SIZE - ( m_pUsed - m_pStart );
   if ( nAvailable < nBytes )
   {
      ZeroMemory( m_pUsed, nAvailable );
      nBytes -= nAvailable;
      m_pUsed += nAvailable;

      return FALSE;
   }

   ZeroMemory( m_pUsed, nBytes );
   m_pUsed += nBytes;
   return TRUE;
}

BOOL cMessage::cBlock::getData ( size_t &nOffset, size_t &nLength, BYTE * &pBuffer ) const
{
   int nAvailable = BUFFER_BLOCK_SIZE - nOffset;

   if ( nAvailable < nLength )
   {
      CopyMemory( pBuffer, m_pStart + nOffset, nAvailable );

      nOffset = 0;
      nLength -= nAvailable;
      pBuffer += nAvailable;

      return FALSE;
   }

   CopyMemory( pBuffer, m_pStart + nOffset, nLength );
   return TRUE;
}

cMessage::cMessage()
{
   // Immediately allocate one block
   m_blocks.push_back ( cBlock() );
}

void cMessage::pasteData ( BYTE *pbData, int nBytes )
{
   while ( !m_blocks.back().pasteData ( pbData, nBytes ) )
      m_blocks.push_back ( cBlock() );
}

void cMessage::pasteAlign ( int nBoundary )
{
   int nBytes = m_blocks.back().getAlign ( nBoundary );
   if ( nBytes != 0 )
   {
      while ( !m_blocks.back().pasteAlign ( nBytes ) )
         m_blocks.push_back ( cBlock() );
   }
}

void cMessage::pasteString ( const char *str )
{
   if( str == NULL )
   {
      // Paste an empty string, 1 char (null) and then dword align
      paste ( 1L );
      return;
   }

   int nLength = lstrlen( str ) + 1;
   paste ( static_cast< short > ( nLength ) );
   pasteData ( reinterpret_cast< BYTE * > ( const_cast< char * > ( str ) ), nLength );
   pasteAlign ( sizeof ( long ) );
}

size_t cMessage::getSize () const
{
   return ( m_blocks.size () - 1 ) * BUFFER_BLOCK_SIZE + m_blocks.back().getUsed ();
}

void cMessage::getData ( size_t nOffset, size_t nLength, BYTE *pBuffer ) const
{
   nOffset %= BUFFER_BLOCK_SIZE;
   for ( cBlockList::const_iterator i = m_blocks.begin() + ( nOffset / BUFFER_BLOCK_SIZE );
      !i->getData ( nOffset, nLength, pBuffer); ++ i );
#if _DEBUG
	//char szUpdate[50];
//	sprintf(szUpdate,"nOffset: %d",nOffset);
//	UpdateConsole((char *)szUpdate);
#endif
}

cMessage::cBlockDispenser::cBlockDispenser ()
: m_hHeap( ::HeapCreate ( 0, 0, 0 ) )
{
}

cMessage::cBlockDispenser::~cBlockDispenser ()
{
   ::HeapDestroy( m_hHeap );
}

BYTE *cMessage::cBlockDispenser::get ()
{
   return reinterpret_cast< BYTE * > ( ::HeapAlloc ( m_hHeap, 0, BUFFER_BLOCK_SIZE ) );
}

void cMessage::cBlockDispenser::release ( BYTE *pBlock )
{
   ::HeapFree ( m_hHeap, 0, pBlock );
}

cMessage::cBlockDispenser cMessage::g_dispenser;