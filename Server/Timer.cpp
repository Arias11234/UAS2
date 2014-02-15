// Timer.cpp: implementation of the Timer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Timer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Timer::Timer()
{
	m_bStop=true;
	m_msTimeout=-1;
	m_hThreadDone = NULL;
	m_hThreadDone = CreateEvent(NULL,FALSE, FALSE, NULL);
	ASSERT (m_hThreadDone);
	SetEvent(m_hThreadDone);
	 // code added
	m_hEndThread = NULL;
	m_hEndThread = CreateEvent(NULL,FALSE, FALSE, NULL);
	ASSERT (m_hEndThread);
	SetEvent(m_hEndThread);

}

Timer::~Timer()
{
	//dont destruct until the thread is done
	DWORD ret=WaitForSingleObject(m_hEndThread,INFINITE);
	ASSERT(ret==WAIT_OBJECT_0);
	Sleep(500);
	CloseHandle(m_hEndThread) ;
}

void Timer::Tick()
{
	//Will be overriden by subclass

}

void Timer::StartTicking()
{
	if (m_bStop==false)
		return; ///ignore, it is already ticking...
	m_bStop=false;
	ResetEvent(m_hThreadDone);
	ResetEvent(m_hEndThread);
	AfxBeginThread(TickerThread, this);
}

UINT Timer::TickerThread(LPVOID pParam)
{
	Timer* me=(Timer*) pParam;
	ASSERT (me->m_msTimeout!=-1);
	while (!me->m_bStop)
	{
		// Sleep() is changed to this so that a termination of a thread will be received.
		if (WaitForSingleObject(me->m_hEndThread,me->GetTimeout()) == WAIT_TIMEOUT)
		{
			// No signal received..
			me->Tick(); 
		}
		else
		{
			// Signal received to stop the thread.
			me->m_bStop = true;
		}
	}	
	SetEvent(me->m_hThreadDone);
	return 0;
}

void Timer::StopTicking()
{
	if (m_bStop==true)
		return; ///ignore, it is not ticking...

	m_bStop=true; //ok make it stop
	SetEvent(m_hEndThread);
	WaitForSingleObject(m_hThreadDone,INFINITE); 
	//The above ensures that we do not return UNTIL the thread
	//has finished. This way we dont allow the user to start multiple
	//threads that will execute Tick() at the same time

}
