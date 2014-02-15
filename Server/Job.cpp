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
 *	@file Job.cpp
 *	Implements functionality for a job pool.
 *
 *	Used for objects that require scheduling.
 */

#include "Job.h"

clock_t  cJobPool::longtime;
clock_t  cJobPool::NextTickTime;

int cJob::Tick( )
{
	if ( m_fEnabled )
	{
		if ( ++m_iCounter == m_iFrequency )
		{
			m_iCounter = 0;
			int ret = m_mcFunction( m_wParam, m_lParam );
			//UpdateConsole( " Yep.\r\n");
			if ( --m_iNumTimes == 0 )
				return JOB_REMOVE;
			else
				return ret;
		}
	}
	return -1;
}

void cJobPool::Tick( )
{
	std::list< cJob * >::iterator itJob = m_lstJobList.begin( );
	for ( ; itJob != m_lstJobList.end( ); ++itJob)
	{
		//char szMessage[100];
		//sprintf (szMessage, "Job: %u.\r\n", (*itJob)->GetJobID( ) );
		//UpdateConsole( (char *)szMessage);
		switch ( (*itJob)->Tick( ) )
		{
		case JOB_DISABLE:
			(*itJob)->Enable( FALSE );
			break;	// case JOB_DISABLE
		
		case JOB_NORMAL:
			break;	// case JOB_NORMAL
		
		case JOB_REMOVE:
			//RemoveJob( (*itJob)->GetJobID( ) );
			SAFEDELETE( *itJob )
			itJob = m_lstJobList.erase( itJob );
			break;	// case JOB_REMOVE
		}
	}
}

int	cJobPool::CreateJob( MessageCall mc, LPVOID wParam, LPVOID lParam, char *szName, int iFreq, int iNumTimes )
{
	cJob *pcNewJob = new cJob( mc, wParam, lParam, iFreq, iNumTimes, szName, m_iJobID++ );
	m_lstJobList.push_back( pcNewJob );
	
	return m_iJobID - 1;
}

int cJobPool::GetJobIDByName( char *szName )
{
	std::list< cJob * >::iterator itJob = m_lstJobList.begin( );
	for ( ; itJob != m_lstJobList.end( ); ++itJob )
	{
		if ( lstrcmpi( (*itJob)->GetName( ), szName ) == 0 )
			return (*itJob)->GetJobID( );
	}

	return -1;
}

cJob *cJobPool::GetJobByID( int iID )
{
	std::list< cJob * >::iterator itJob = m_lstJobList.begin( );
	for ( ; itJob != m_lstJobList.end( ); ++itJob )
	{
		if ( (*itJob)->GetJobID( ) == iID )
			return (*itJob);
	}

	return NULL;
}

void cJobPool::RemoveJob( int iID )
{
	std::list< cJob * >::iterator itJob = m_lstJobList.begin( );
	for ( ; itJob != m_lstJobList.end( ); ++itJob )
	{
		if ( (*itJob)->GetJobID( ) == iID )
		{
			SAFEDELETE( *itJob )
			m_lstJobList.erase( itJob );
			break;
		}
	}
}