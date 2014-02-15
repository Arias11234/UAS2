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
 *	@file WinMain.cpp
 *	Implements functionality for the application GUI and console.
 */

#include <winsock2.h>
#include "resource.h"
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include "VersionNo.h"
#include "calc_funcs.h"
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include "MasterServer.h"
#include "WorldManager.h"
#include "Status.h"

#define MYWM_NOTIFYICON (WM_APP+100)
#define CONSOLE_WIDTH_NORM  218
#define CONSOLE_WIDTH_OPT	610
#define CONSOLE_HEIGHT_NORM 535
#define CONSOLE_HEIGHT_OPT  535

HINSTANCE	g_hInstance;
HWND		g_hWndMain;
HWND		g_hWndConsole;
HWND		g_hWndHelpTitle;
HWND		g_hWndHelp;
BOOL		g_fStarted;
BOOL		g_fConfig;
BOOL		g_fClients;

short	g_nWorldPort;
short	g_nCharPort;
char	g_szLocalIP[16];

char    g_szDBIP[16];
char    g_szDBNAME[20];
char	g_szDBUSER[20];
char	g_szDBPASSWORD[20];
int		g_DBType;

int GetLocalAddress( LPSTR lpStr, LPDWORD lpdwStrLen )
{
    struct in_addr *pinAddr;
    LPHOSTENT	lpHostEnt;
	int			iRet;
	int			iLen;

    iRet = gethostname( lpStr, *lpdwStrLen );
	if ( iRet == SOCKET_ERROR )
	{
		lpStr[0] = '\0';
		return SOCKET_ERROR;
	}

	lpHostEnt = gethostbyname( lpStr );
    if ( lpHostEnt == NULL )
	{
		lpStr[0] = '\0';
		return SOCKET_ERROR;
	}

	pinAddr = ((LPIN_ADDR)lpHostEnt->h_addr);
	iLen = lstrlen( inet_ntoa( *pinAddr ) );
	if ( (DWORD)iLen > *lpdwStrLen )
	{
		*lpdwStrLen = iLen;
		WSASetLastError( WSAEINVAL );
		return SOCKET_ERROR;
	}

	*lpdwStrLen = iLen;
	lstrcpy( lpStr, inet_ntoa( *pinAddr ) );

    return 0;
}

void UpdateConsole( const char *szBuff )
{
	int iLength = SendMessage( g_hWndConsole, WM_GETTEXTLENGTH, 0, 0 );

//	SendMessage( g_hWndConsole, EM_SETSEL, iLength, iLength );
//	SendMessage( g_hWndConsole, EM_REPLACESEL, FALSE, (LPARAM)szBuff );
	
	if(iLength < 5000){
		SendMessage( g_hWndConsole, EM_SETSEL, iLength, iLength );
		SendMessage( g_hWndConsole, EM_REPLACESEL, FALSE, (LPARAM)szBuff );
	}
	else 
	{
		iLength = 0;
		SendMessage( g_hWndConsole, WM_SETTEXT, iLength, iLength );
		SendMessage( g_hWndConsole, EM_SETSEL, iLength, iLength );
		SendMessage( g_hWndConsole, EM_REPLACESEL, FALSE, (LPARAM)szBuff );
	}
}

long UpdateConsole( char *szBuff, long nErr )
{
	UpdateConsole( szBuff );
	
	if ( !nErr )
	{
	 	char *lpMsgBuf;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, nErr, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR) &lpMsgBuf, 0, NULL );
		UpdateConsole( lpMsgBuf );
		LocalFree( lpMsgBuf );

	}
	else
		UpdateConsole( "success.\r\n\r\n" );

	return nErr;
}


void UpdateConsole( char *szMessage, ... )
{
	char szBuffer[1024];
	va_list	valMaker;
	
	va_start( valMaker, szMessage );
	wvsprintf( (char *)szBuffer, (const char *)szMessage, valMaker );

	int iLength = SendMessage( g_hWndConsole, WM_GETTEXTLENGTH, 0, 0 );

	if(iLength < 5000){
		SendMessage( g_hWndConsole, EM_SETSEL, iLength, iLength );
		SendMessage( g_hWndConsole, EM_REPLACESEL, FALSE, (LPARAM)szBuffer );
	}
	else 
	{
		iLength = 0;
		SendMessage( g_hWndConsole, WM_SETTEXT, iLength, iLength );
		SendMessage( g_hWndConsole, EM_SETSEL, iLength, iLength );
		SendMessage( g_hWndConsole, EM_REPLACESEL, FALSE, (LPARAM)szBuffer );
	}


}

void UpdateHelpTitle( char *szMessage, ... )
{
	char szBuffer[1024];
	va_list	valMaker;
	
	va_start( valMaker, szMessage );
	wvsprintf( (char *)szBuffer, (const char *)szMessage, valMaker );

	int iLength = SendMessage( g_hWndHelpTitle, WM_GETTEXTLENGTH, 0, 0 );

	iLength = 0;
	//BUGGY
//	SendMessage( g_hWndHelpTitle, WM_SETTEXT, iLength, iLength );
//	SendMessage( g_hWndHelpTitle, EM_SETSEL, iLength, iLength );
//	SendMessage( g_hWndHelpTitle, EM_REPLACESEL, FALSE, (LPARAM)szBuffer );
}

void UpdateHelp( char *szMessage, ... )
{
	char szBuffer[1024];
	va_list	valMaker;
	
	va_start( valMaker, szMessage );
	wvsprintf( (char *)szBuffer, (const char *)szMessage, valMaker );

	int iLength = SendMessage( g_hWndHelp, WM_GETTEXTLENGTH, 0, 0 );

	iLength = 0;
	SendMessage( g_hWndHelp, WM_SETTEXT, iLength, iLength );
	SendMessage( g_hWndHelp, EM_SETSEL, iLength, iLength );
	SendMessage( g_hWndHelp, EM_REPLACESEL, FALSE, (LPARAM)szBuffer );
}

void SystrayAdd( HWND hWnd )
{
	NOTIFYICONDATA nid;
	memset( &nid, 0, sizeof( NOTIFYICONDATA ) );
	nid.cbSize = sizeof( NOTIFYICONDATA );
	nid.hWnd = hWnd;
	nid.uID = ID_SYSTRAY;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = MYWM_NOTIFYICON;
	nid.hIcon = LoadIcon( g_hInstance, MAKEINTRESOURCE( IDI_ICON ) );
	_tcscpy( nid.szTip, "UAS2 Server" );
	Shell_NotifyIcon( NIM_ADD, &nid );
	DestroyIcon( nid.hIcon );
}

void SystrayDelete( HWND hWnd )
{
	NOTIFYICONDATA nid;
	memset( &nid, 0, sizeof( NOTIFYICONDATA ) );
	nid.cbSize = sizeof( NOTIFYICONDATA );
	nid.hWnd = hWnd;
	nid.uID = ID_SYSTRAY;
	Shell_NotifyIcon( NIM_DELETE, &nid );
}

int CALLBACK MainWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	static HWND hWndStart;
	static HWND hWndClearObjs;
	static HWND hWndDiscAll;
	static HWND hWndDBTYPE;
	static HWND hWndDBIP;
	static HWND hWndDBNAME;
	static HWND hWndDBUSER;
	static HWND hWndDBPASSWORD;
	static HWND hWndCONFIG;
	static HWND hWndACCESSFILE;
	static HWND hWndPrivate;
	static HWND hWndUpdate;

	static HWND hWndSettings[30];
	static BOOL fExit = FALSE;
	static HWND hWndExit;
	static UINT s_uTaskbarRestart;
	static BOOL fMinimized = FALSE;

	switch ( msg )
	{	
		case WM_INITDIALOG:
		{
			HICON hIcon;
			hIcon = LoadIcon( g_hInstance, MAKEINTRESOURCE( IDI_ICON ) );
			SendMessage( hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon );
			SendMessage( hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon );
			DeleteObject( hIcon );

			s_uTaskbarRestart = RegisterWindowMessage( TEXT( "TaskbarCreated" ) );

			g_hWndConsole	= GetDlgItem( hWnd, IDC_CONSOLE );
			g_hWndHelpTitle	= GetDlgItem( hWnd, IDC_HELP_TITLE );
			g_hWndHelp		= GetDlgItem( hWnd, IDC_HELP_TEXT );
			hWndDBTYPE		= GetDlgItem( hWnd, IDC_DBTYPE );
			hWndDBIP		= GetDlgItem( hWnd, IDC_DBIP );
			hWndDBNAME		= GetDlgItem( hWnd, IDC_DBNAME );
			hWndDBUSER		= GetDlgItem( hWnd, IDC_DBUSER );
			hWndDBPASSWORD	= GetDlgItem( hWnd, IDC_DBPASSWORD );
			hWndACCESSFILE	= GetDlgItem( hWnd, IDC_ACCESS_FILE );
			hWndCONFIG		= GetDlgItem( hWnd, IDC_CONFIG );
			hWndPrivate		= GetDlgItem( hWnd, IDC_SERVER );
			hWndUpdate		= GetDlgItem( hWnd, IDC_BTN_UPDATE );

			hWndSettings[0]	= GetDlgItem( hWnd, IDC_STATIP );
			hWndSettings[1]	= GetDlgItem( hWnd, IDC_STATCP );
			hWndSettings[2]	= GetDlgItem( hWnd, IDC_STATWP );
			hWndSettings[3]	= GetDlgItem( hWnd, IDC_STATDBIP );
			hWndSettings[4]	= GetDlgItem( hWnd, IDC_STATDBN );
			hWndSettings[5]	= GetDlgItem( hWnd, IDC_STATDBU );
			hWndSettings[6]	= GetDlgItem( hWnd, IDC_STATDBPASS );
			hWndSettings[7]	= GetDlgItem( hWnd, IDC_LOCALIP );
			hWndSettings[8]	= GetDlgItem( hWnd, IDC_CHARPORT );
			hWndSettings[9]	= GetDlgItem( hWnd, IDC_WORLDPORT );
			hWndSettings[10]	= GetDlgItem( hWnd, IDC_MYSQL_DB );
			hWndSettings[11]	= GetDlgItem( hWnd, IDC_MSSQL_DB );
			hWndSettings[12]	= GetDlgItem( hWnd, IDC_ACCESS_DB );
			hWndSettings[13]	= GetDlgItem( hWnd, IDC_SETTING_FRAME );
			hWndSettings[14]	= GetDlgItem( hWnd, IDC_FRAME_DB );
			hWndSettings[15]	= GetDlgItem( hWnd, IDC_STAT_MAXUSERS );
			hWndSettings[16]	= GetDlgItem( hWnd, IDC_MAX_USERS );
			hWndSettings[17]	= GetDlgItem( hWnd, IDC_STAT_SNAME );
			hWndSettings[18]	= GetDlgItem( hWnd, IDC_ED_SNAME );
							
			WSADATA		wsaData;
			struct hostent *host;
			USHORT		wVersionRequested = 0x0202;
 
			UpdateConsole( " Initializing Winsock 2.0 ... ", !WSAStartup( wVersionRequested, &wsaData ) );
			
			host = NULL;
/*
			////////// Read in Data
			FILE *pcStatConfig = fopen( "status.ini","rt" );

			if ( pcStatConfig )
			{
				char line[100];

				while ( !feof( pcStatConfig ) )
				{
					fgets( line, 100, pcStatConfig );
					char temp[100];
					char strData[50];
					char value[50];

					memcpy(temp,line,sizeof(line));
					char* pszSepTemp = strchr(temp, (int)'=');

					if(pszSepTemp == NULL)
					{
					}
					else
					{
						*pszSepTemp = '\0';
						sprintf(strData,"%s",temp);
					}

					char* pszSep = strchr(line, (int)'=');	

					if(pszSep == NULL)
					{
						// No data
					}
					else
					{
						*pszSep = '\0';
						++pszSep;
						char* pszValue = strchr(pszSep, (int)';');
						if(pszValue == NULL){
						}
						else
						{
							*pszValue = '\0';
						}
					
						sprintf(value,"%s",pszSep);
						
						if (lstrcmpi(strData,"Server") == 0)
							{
								if( lstrcmpi(value,"Private") == 0)
								{
									UpdateConsole(" Server: Private\r\n");
									cMasterServer::cStatus->m_fPrivate = 1;
								}
								else
								{
									UpdateConsole(" Server: Public\r\n");
									cMasterServer::cStatus->m_fPrivate = 0;
								}
							}
						else if(lstrcmpi(strData,"Host") == 0)
							{
								if(isalpha(value[0]))
								{
									//Do DNS Lookup for IP
									host = gethostbyname(value);
									memcpy(cMasterServer::cStatus->m_strHost,inet_ntoa(*(struct in_addr *) host->h_addr_list[0]),16);
								}
								else
								{
									memcpy(cMasterServer::cStatus->m_strHost,value,sizeof(value));
								}	
							}
						else if(lstrcmpi(strData,"Port") == 0)
							{
								cMasterServer::cStatus->m_sPort = atoi(value);
							}
						else if(lstrcmpi(strData,"Path") == 0)
							{
								memcpy(cMasterServer::cStatus->m_strHTTP,value,sizeof(value));
							}					
						else if(lstrcmpi(strData,"ID") == 0)
							{
								cMasterServer::cStatus->m_sID = atol(value);
							}
						else if(lstrcmpi(strData,"Serial") == 0)
							{
								memcpy(cMasterServer::cStatus->m_sSer,value,sizeof(value));
							}
						else if(lstrcmpi(strData,"Key") == 0)
							{
								memcpy(cMasterServer::cStatus->m_sKey,value,sizeof(value));
							}
						else if(lstrcmpi(strData,"Client") == 0)
							{
								memcpy(cMasterServer::cStatus->m_cVersion,value,sizeof(value));
							}
						else if(lstrcmpi(strData,"MaxUsers") == 0)
							{
								cMasterServer::cStatus->m_dwMax = atol(value);
							}
						else
							{
							//	UpdateConsole("Error in status.ini   %s\r\n", strData);
							}
					}
				}
				fclose( pcStatConfig );
			}
			else
			{
				cMasterServer::cStatus->m_fPrivate = true;

			}
*/
			cMasterServer::cStatus->m_fPrivate = true;

			//////////////////
			SetDlgItemText( hWnd, IDC_VERSIONTEXT, SERVERVERSION);
			SetDlgItemText( hWnd, IDC_VERSIONTEXT2, STRFILEVER);

			DWORD dwLength;
			DWORD dwType = REG_SZ;
			char szTemp[5];
			char szDBType[2];
			char szStatusTemp[20];
			char szHostTemp[64];

			HKEY hKey;
			RegCreateKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\UAS", NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL );
		
			////////////////////////
			// Server Status Settings
			dwLength = sizeof( szStatusTemp );
			if ( ( RegQueryValueEx( hKey, "Private", NULL, &dwType, (BYTE*)&szStatusTemp, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				cMasterServer::cStatus->m_fPrivate = atoi(szStatusTemp);
				CheckDlgButton( hWnd, IDC_SERVER, cMasterServer::cStatus->m_fPrivate );
			}
			else
			{
				CheckDlgButton( hWnd, IDC_SERVER, 1 );
				cMasterServer::cStatus->m_fPrivate = 1;
			}
			
			dwLength = sizeof( szHostTemp );
			if ( ( RegQueryValueEx( hKey, "Status Host", NULL, &dwType, (BYTE*)&szHostTemp, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				if(isalpha(szHostTemp[0]))
				{
					//Do DNS Lookup for IP
					host = gethostbyname(szHostTemp);
					memcpy(cMasterServer::cStatus->m_strHost,inet_ntoa(*(struct in_addr *) host->h_addr_list[0]),16);
					SetDlgItemText( hWnd, IDC_ED_HOST, szHostTemp );
				}
				else
				{
					memcpy(cMasterServer::cStatus->m_strHost,szHostTemp,sizeof(szHostTemp));
					SetDlgItemText( hWnd, IDC_ED_HOST, szHostTemp );
				}
			}
			else
			{
				SetDlgItemText( hWnd, IDC_ED_HOST, "127.0.0.1" );
				memcpy(cMasterServer::cStatus->m_strHost,"127.0.0.1",10);
			}
						
			dwLength = sizeof( szTemp );
			if ( ( RegQueryValueEx( hKey, "HTTP Port", NULL, &dwType, (BYTE*)&szTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_ED_PORT, szTemp );
				cMasterServer::cStatus->m_sPort = atoi(szTemp);
			}
			else
			{
				SetDlgItemText( hWnd, IDC_ED_PORT, "80" );
				cMasterServer::cStatus->m_sPort = 80;
			}

			dwLength = sizeof( szTemp );
			if ( ( RegQueryValueEx( hKey, "Server ID", NULL, &dwType, (BYTE*)&szTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_ED_ID, szTemp );
				cMasterServer::cStatus->m_sID = atol(szTemp);
			}
			else
			{
				SetDlgItemText( hWnd, IDC_ED_ID, "0" );
				cMasterServer::cStatus->m_sID = 0;
			}

			dwLength = sizeof( szHostTemp );
			if ( ( RegQueryValueEx( hKey, "Server Serial", NULL, &dwType, (BYTE*)&szHostTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_ED_SERIAL, szHostTemp );
				memcpy(cMasterServer::cStatus->m_sSer,szHostTemp,sizeof(szHostTemp));
			}
			else
			{
				SetDlgItemText( hWnd, IDC_ED_SERIAL, "0" );
				memcpy(cMasterServer::cStatus->m_sSer,"0",2);
			}
			
			dwLength = sizeof( szHostTemp );
			if ( ( RegQueryValueEx( hKey, "Server Key", NULL, &dwType, (BYTE*)&szHostTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_ED_KEY, szHostTemp );
				memcpy(cMasterServer::cStatus->m_sKey,szHostTemp,sizeof(szHostTemp));
			}
			else
			{
				SetDlgItemText( hWnd, IDC_ED_KEY, "000000" );
				memcpy(cMasterServer::cStatus->m_sKey,"000000",7);
			}

			dwLength = sizeof( szHostTemp );
			if ( ( RegQueryValueEx( hKey, "Client Support", NULL, &dwType, (BYTE*)&szHostTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_ED_CLIENT, szHostTemp );
				memcpy(cMasterServer::cStatus->m_cVersion,szHostTemp,sizeof(szHostTemp));
			}
			else
			{
				SetDlgItemText( hWnd, IDC_ED_CLIENT, "53" );
				memcpy(cMasterServer::cStatus->m_cVersion,"53",3);
			}
						
			dwLength = sizeof( szHostTemp );
			if ( ( RegQueryValueEx( hKey, "HTTP Path", NULL, &dwType, (BYTE*)&szHostTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_ED_URL, szHostTemp );
				memcpy(cMasterServer::cStatus->m_strHTTP,szHostTemp,sizeof(szHostTemp));
			}
			else
			{
				SetDlgItemText( hWnd, IDC_ED_URL, "/" );
				memcpy(cMasterServer::cStatus->m_strHTTP,"/",2);
			}
			
			////////////////////////
			// Server Settings
			// Server Name
			dwLength = sizeof( szHostTemp );
			if ( ( RegQueryValueEx( hKey, "Server Name", NULL, &dwType, (BYTE*)&szHostTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_STAT_NAME, szHostTemp );
				SetDlgItemText( hWnd, IDC_ED_SNAME, szHostTemp );
				memcpy(cMasterServer::m_szServerName,szHostTemp,sizeof(szHostTemp));
			}
			else
			{
				SetDlgItemText( hWnd, IDC_ED_SNAME, "World Name" );
				SetDlgItemText( hWnd, IDC_STAT_NAME, "World Name" );
				sprintf(cMasterServer::m_szServerName,"World Name" );
			}

			dwLength = sizeof( szHostTemp );
			if ( ( RegQueryValueEx( hKey, "Max Clients", NULL, &dwType, (BYTE*)&szHostTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_MAX_USERS, szHostTemp );
				cMasterServer::cStatus->m_dwMax = atol(szHostTemp);
			}
			else
			{
				SetDlgItemText( hWnd, IDC_MAX_USERS, "0" );
				cMasterServer::cStatus->m_dwMax = 0x0L;
			}

			//Character Server Port
			dwLength = sizeof( szTemp );
			if ( ( RegQueryValueEx( hKey, "CharPort", NULL, &dwType, (BYTE*)&szTemp, &dwLength) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
				SetDlgItemText( hWnd, IDC_CHARPORT, szTemp );
			else
				SetDlgItemText( hWnd, IDC_CHARPORT, "9002" );

			// World Server Port
			dwLength = sizeof( szTemp );
			if ( ( RegQueryValueEx( hKey, "WorldPort", NULL, &dwType, (BYTE*)&szTemp, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
				SetDlgItemText( hWnd, IDC_WORLDPORT, szTemp );
			else
				SetDlgItemText( hWnd, IDC_WORLDPORT, "9004" );
			
			/////////////
			// Database Settings
	
			dwLength = sizeof( szDBType );
			if ( ( RegQueryValueEx( hKey, "DBType", NULL, &dwType, (BYTE*)&szDBType, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{	// Set Radio button depending on Value
				SetDlgItemText( hWnd, IDC_DBTYPE, szDBType );
				g_DBType = szDBType[0] - 0x30;
				switch(g_DBType)
				{
					case 1:
					{
						CheckDlgButton( hWnd, IDC_ACCESS_DB, 1 );
						CheckDlgButton( hWnd, IDC_MSSQL_DB, 0 );
						CheckDlgButton( hWnd, IDC_MYSQL_DB, 0 );
									
						// Disable dialog boxes
						EnableWindow( hWndDBIP, FALSE );
						EnableWindow( hWndDBNAME, FALSE );
						EnableWindow( hWndDBUSER, FALSE );
						EnableWindow( hWndDBPASSWORD, FALSE );
						break;
					}
					case 2:
					{					
						CheckDlgButton( hWnd, IDC_ACCESS_DB, 0 );
						CheckDlgButton( hWnd, IDC_MSSQL_DB, 1 );
						CheckDlgButton( hWnd, IDC_MYSQL_DB, 0 );
						
						// Enable dialog boxes
						EnableWindow( hWndDBIP, TRUE );
						EnableWindow( hWndDBNAME, TRUE );
						EnableWindow( hWndDBUSER, TRUE );
						EnableWindow( hWndDBPASSWORD, TRUE );
						break;
					}
					case 3:
					{
						CheckDlgButton( hWnd, IDC_ACCESS_DB, 0 );
						CheckDlgButton( hWnd, IDC_MSSQL_DB, 0 );
						CheckDlgButton( hWnd, IDC_MYSQL_DB, 1 );
						
						// Enable dialog boxes
						EnableWindow( hWndDBIP, TRUE );
						EnableWindow( hWndDBNAME, TRUE );
						EnableWindow( hWndDBUSER, TRUE );
						EnableWindow( hWndDBPASSWORD, TRUE );
						break;
					}

					default:
					{
						CheckDlgButton( hWnd, IDC_ACCESS_DB, 0 );
						CheckDlgButton( hWnd, IDC_MSSQL_DB, 0 );
						CheckDlgButton( hWnd, IDC_MYSQL_DB, 1 );
									
						// Enable dialog boxes
						EnableWindow( hWndDBIP, TRUE );
						EnableWindow( hWndDBNAME, TRUE );
						EnableWindow( hWndDBUSER, TRUE );
						EnableWindow( hWndDBPASSWORD, TRUE );
						break;
					}
				}
			}
			else
			{
				// DBType = Default -- MySQL db
				CheckDlgButton( hWnd, IDC_ACCESS_DB, 0 );
				CheckDlgButton( hWnd, IDC_MSSQL_DB, 0 );
				CheckDlgButton( hWnd, IDC_MYSQL_DB, 1 );
			}

			dwLength = sizeof( g_szDBIP );
			if ( ( RegQueryValueEx( hKey, "DBIP", NULL, &dwType, (BYTE*)&g_szDBIP, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
				SetDlgItemText( hWnd, IDC_DBIP, g_szDBIP );
			else
				SetDlgItemText( hWnd, IDC_DBIP, "0.0.0.0" );
			
			dwLength = sizeof( g_szDBNAME );
			if ( ( RegQueryValueEx( hKey, "DBNAME", NULL, &dwType, (BYTE*)&g_szDBNAME, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
				SetDlgItemText( hWnd, IDC_DBNAME, g_szDBNAME );
			else
				SetDlgItemText( hWnd, IDC_DBNAME, "uas2" );
			
			dwLength = sizeof( g_szDBUSER );
			if ( ( RegQueryValueEx( hKey, "DBUSER", NULL, &dwType, (BYTE*)&g_szDBUSER, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
				SetDlgItemText( hWnd, IDC_DBUSER, g_szDBUSER );
			else
				SetDlgItemText( hWnd, IDC_DBUSER, "uas2" );
			
			dwLength = sizeof( g_szDBPASSWORD );
			if ( ( RegQueryValueEx( hKey, "DBPASSWORD", NULL, &dwType, (BYTE*)&g_szDBPASSWORD, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
				SetDlgItemText( hWnd, IDC_DBPASSWORD, g_szDBPASSWORD );
			else
				SetDlgItemText( hWnd, IDC_DBPASSWORD, "" );
//////////////////////////////////////////////////////////////////

			// Local IP
			dwLength = sizeof( g_szLocalIP );
			if ( ( RegQueryValueEx( hKey, "LocalIP", NULL, &dwType, (BYTE*)&g_szLocalIP, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
				SetDlgItemText( hWnd, IDC_LOCALIP, g_szLocalIP );
			else
			{
				char szLocalHostName[80];
				DWORD dwSize = sizeof( szLocalHostName );
				if ( GetLocalAddress( szLocalHostName, &dwSize ) == 0 )
					SetDlgItemText( hWnd, IDC_LOCALIP, szLocalHostName );
				else
					SetDlgItemText( hWnd, IDC_LOCALIP, "127.0.0.1" );
			}
	
			// Access Database 
			char	szDirBuff[MAX_PATH+1];
			dwLength = sizeof( cWorldManager::g_szAccessFile );
			if ( ( RegQueryValueEx( hKey, "ACCESSMDB", NULL, &dwType, (BYTE*)&cWorldManager::g_szAccessFile, &dwLength ) == ERROR_SUCCESS ) && ( dwLength > 0 ) )
			{
				SetDlgItemText( hWnd, IDC_ACCESS_FILE, cWorldManager::g_szAccessFile );
			}
			else
			{
				int index = GetCurrentDirectory(MAX_PATH, szDirBuff);
				sprintf(cWorldManager::g_szAccessFile,"%s\\UAS2.mdb", szDirBuff );
				SetDlgItemText( hWnd, IDC_ACCESS_FILE,cWorldManager::g_szAccessFile );
			}

			RegCloseKey( hKey );

			hWndStart = GetDlgItem( hWnd, IDB_START );
			hWndClearObjs = GetDlgItem( hWnd, IDB_CLEAROBJECTS );
			hWndDiscAll = GetDlgItem( hWnd, IDB_DISC_ALL );
			hWndExit = GetDlgItem( hWnd, IDB_EXIT );

			/////// Server Status Data ///////////////////////////////////////////////
			char szPort[5];
			char szTempIP[16];
			SOCKADDR_IN	saServer;

			GetDlgItemText( hWnd, IDC_LOCALIP, szTempIP, sizeof( szTempIP ) );
			saServer.sin_addr.s_addr	= inet_addr( szTempIP );
		
			cMasterServer::cStatus->m_bServer[0] = saServer.sin_addr.S_un.S_un_b.s_b1;
			cMasterServer::cStatus->m_bServer[1] = saServer.sin_addr.S_un.S_un_b.s_b2;
			cMasterServer::cStatus->m_bServer[2] = saServer.sin_addr.S_un.S_un_b.s_b3;
			cMasterServer::cStatus->m_bServer[3] = saServer.sin_addr.S_un.S_un_b.s_b4;

			GetDlgItemText( hWnd, IDC_CHARPORT, szPort, sizeof( szPort ) );
			cMasterServer::cStatus->m_ServerPort = atoi( szPort );
			///////////////////////////////////////////////////////////////////////////
			MoveWindow( hWnd,0 , 0 , CONSOLE_WIDTH_NORM ,CONSOLE_HEIGHT_NORM , TRUE );

			cMasterServer::cStatus->ServerLoad();

			break;	// case WM_INITDIALOG
		}

		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO( lParam )->ptMinTrackSize.x = 340;
			LPMINMAXINFO( lParam )->ptMinTrackSize.y = CONSOLE_HEIGHT_NORM;//150
			break;	// case WM_GETMINMAXINFO
		}

		case WM_SIZE:
		{
			MoveWindow( g_hWndConsole, 8, 95, LOWORD( lParam ) - 15, HIWORD( lParam ) - 100, TRUE );

			if( wParam == SIZE_MINIMIZED )
			{
				SystrayAdd( hWnd );
				ShowWindow( hWnd, SW_HIDE );
				fMinimized = TRUE;
			}
			else if( wParam == SIZE_RESTORED )
			{
				SystrayDelete( hWnd );
				fMinimized = FALSE;	
			}

			break;	// case WM_SIZE
		}

		case WM_NOTIFY:
		{
			break;	// case WM_NOTIFY
		}

		case WM_COMMAND:
		{	
			switch ( GET_WM_COMMAND_ID( wParam, lParam ) )
			{
				case IDC_LOCALIP:
					{
						UpdateHelpTitle( "Local IP:\r\n");
						UpdateHelp( "The IP address or DNS name of the hosting server. The default value is the IP address 127.0.0.1 (the loopback IP address).\r\n");
						break;
					}
				case IDC_SERVER:
					{
						UpdateHelpTitle( "Private Server Checkbox:\r\n");
						UpdateHelp( "When checked, server will not send updates to the status website. The option does not affect how the server operates.\r\n");
						break;
					}
				case IDC_CHARPORT:
					{
						UpdateHelpTitle( "Character Server Port:\r\n");
						UpdateHelp( "The number of the TCP or UDP port used to connect to the Character Server. The default value is 9002.\r\n");
						break;
					}
				case IDC_MAX_USERS:
					{
						UpdateHelpTitle( "Maximum Clients:\r\n");
						UpdateHelp( "The maximum number of clients the server allows. Not implemented.\r\n");
						break;
					}
				case IDC_WORLDPORT:
					{
						UpdateHelpTitle( "World Server Port:\r\n");
						UpdateHelp( "The number of the TCP or UDP port used to connect to the World Server. The default value is 9004.\r\n");
						break;
					}
				case IDC_ED_SNAME:
					{
						UpdateHelpTitle( "World Name:\r\n");
						UpdateHelp( "The name used to identify the world hosted by this server. Appears in the MOTD during login process.\r\n");
						break;
					}

				case IDC_ACCESS_FILE:
					{
						UpdateHelpTitle( "Microsoft Access File:\r\n");
						UpdateHelp( "The full path to the UAS2 Microsoft Access (.mdb) database file. The default path is the working directory.\r\n");
						break;
					}
				case IDC_DBIP:
					{
						UpdateHelpTitle( "Database IP:\r\n");
						UpdateHelp( "The IP address of the database server.  The value must be an IP address and not a DNS name.\r\n");
						break;
					}
				case IDC_DBNAME:
					{
						UpdateHelpTitle( "Database Name:\r\n");
						UpdateHelp( "The name of the UAS2 database.\r\n");
						break;
					}
				case IDC_DBUSER:
					{
						UpdateHelpTitle( "Database User:\r\n");
						UpdateHelp( "A username with permission to access the database.\r\n");
						break;
					}
				case IDC_DBPASSWORD:
					{
						UpdateHelpTitle( "Database Password:\r\n");
						UpdateHelp( "The password for the corresponding username.\r\n");
						break;
					}

				case IDC_ED_HOST:
					{
						UpdateHelpTitle( "State Website Host:\r\n");
						UpdateHelp( "The IP address or DNS name of the website hosting the server status pages.\r\n");
						break;
					}
				case IDC_ED_URL:
					{
						UpdateHelpTitle( "URL Path:\r\n");
						UpdateHelp( "The URL path to the server status webpage. Must start with '/' and end with '/'.\r\n");
						break;
					}
				case IDC_ED_PORT:
					{
						UpdateHelpTitle( "Access Port:\r\n");
						UpdateHelp( "The number of the TCP or UDP port used to access the server status website. The default port number is 80 (HTTP).\r\n");
						break;
					}
				case IDC_ED_ID:
					{
						UpdateHelpTitle( "Server ID:\r\n");
						UpdateHelp( "The server ID for the server status listing. Supplied by the host website during registration.\r\n");
						break;
					}
				case IDC_ED_KEY:
					{
						UpdateHelpTitle( "Key ID:\r\n");
						UpdateHelp( "The key ID for the server status listing. Supplied by the host website during registration.\r\n");
						break;
					}
				case IDC_ED_CLIENT:
					{
						UpdateHelpTitle( "Client Version:\r\n");
						UpdateHelp( "Specifies which client(s) are supported with this server. Supplied by the host website during registration.\r\n");
						break;
					}
				case IDC_ED_SERIAL:
					{
						UpdateHelpTitle( "Serial Code:\r\n");
						UpdateHelp( "The serial code for your server to use when connecting to the server status host website. Supplied by the host website during registration.\r\n");
						break;
					}

				case IDC_WBBOX:
					{
						UpdateHelpTitle( "World Broadcast:\r\n");
						UpdateHelp( "A message that may be sent to all users presently connected to the world.\r\n");
						break;
					}

				case IDB_EXIT:
					{
						UpdateConsole( "\r\n Exiting server. Please wait ...\r\n" );
						cMasterServer::cStatus->ServerOffline();
						PostMessage( hWnd, WM_CLOSE, 0, 0 );
						EnableWindow( hWndExit, FALSE );
						break;	// case IDB_EXIT
					}

				case IDB_CLEAROBJECTS:
					if ( g_fStarted )
					{
						cMasterServer::ClearAllObjects( );
						UpdateConsole( " All spawned objects cleared!\r\n" );
						break;
					}
					break;

				case IDB_DISC_ALL:
				{
					if( g_fStarted )
					{
						cMasterServer::DisconnectAllClients( );
						UpdateConsole( " All clients disconnected!\r\n" );
						break;
					}
					break;
				}

				//Karki
				case IDC_WB:
					if ( g_fStarted )
					{
						UpdateConsole( " World broadcast sent!\r\n" );
						char szWBTemp[255];
						GetDlgItemText( hWnd, IDC_WBBOX, szWBTemp, sizeof( szWBTemp ) );
						cMasterServer::ServerMessage( ColorGreen, NULL, "%s", szWBTemp );
					}
					break;

				case IDB_START:
				{
					if ( g_fStarted )
					{					
						EnableWindow( hWndStart, FALSE );
						EnableWindow( hWndClearObjs, FALSE );
						EnableWindow( hWndDiscAll, FALSE );
						
						g_fStarted = !cMasterServer::Unload( );

						if( ( fExit ) && ( !g_fStarted ) )
						{
							SystrayDelete( hWnd );
							DestroyWindow( hWnd );
							PostQuitMessage( 0 );
						}
						
						SetWindowText( hWndStart, "&Start Server" );
						EnableWindow( hWndStart, TRUE );
					}
					else
					{
						g_fStarted = TRUE;

						char szTemp[5];

						GetDlgItemText( hWnd, IDC_LOCALIP, g_szLocalIP, sizeof( g_szLocalIP ) );
						GetDlgItemText( hWnd, IDC_DBTYPE, szTemp, sizeof( szTemp ) );
						g_DBType = szTemp[0] - 0x30;
						GetDlgItemText( hWnd, IDC_DBIP, g_szDBIP, sizeof( g_szDBIP ) );
						GetDlgItemText( hWnd, IDC_DBNAME, g_szDBNAME, sizeof( g_szDBNAME ) );
						GetDlgItemText( hWnd, IDC_DBUSER, g_szDBUSER, sizeof( g_szDBUSER ) );
						GetDlgItemText( hWnd, IDC_DBPASSWORD, g_szDBPASSWORD, sizeof( g_szDBPASSWORD ) );
						GetDlgItemText( hWnd, IDC_ACCESS_FILE, cWorldManager::g_szAccessFile, sizeof( cWorldManager::g_szAccessFile ) );

						SetDlgItemText( hWnd, IDB_START, "&Stop Server");
							
						GetDlgItemText( hWnd, IDC_CHARPORT, szTemp, sizeof( szTemp ) );
						g_nCharPort = atoi( szTemp );

						GetDlgItemText( hWnd, IDC_WORLDPORT, szTemp, sizeof( szTemp ) );
						g_nWorldPort = atoi( szTemp );

						cDatabase::SetupDB(g_DBType,g_szDBIP,g_szDBNAME,g_szDBUSER,g_szDBPASSWORD);

						cMasterServer::Load( );

						EnableWindow( hWndClearObjs, TRUE );
						EnableWindow( hWndDiscAll, TRUE );
					}
					break;	// case IDB_START
				}

				case ID_SYSTRAY_ACE:
				{
					ShowWindow( hWnd, SW_RESTORE );
					SetForegroundWindow( hWnd );
					SendMessage( hWnd, WM_ACTIVATEAPP, ( WPARAM )TRUE, ( LPARAM )NULL );
					break;
				}

				case ID_SYSTRAY_EXIT:
				{
					PostMessage( hWnd, WM_CLOSE, 0, 0 );
					break;
				}

				case IDC_CONFIG:
				{
					if(g_fConfig == FALSE)
					{
						UpdateHelpTitle("Configuration Settings:\r\n");
						UpdateHelp("Changes will not take effect until the server program is restarted.\r\n");
						MoveWindow( hWnd,0 , 0 , CONSOLE_WIDTH_OPT ,CONSOLE_HEIGHT_OPT , TRUE );
						ShowWindow( hWndDBIP, SW_SHOW );
						ShowWindow( hWndDBNAME, SW_SHOW );
						ShowWindow( hWndDBUSER, SW_SHOW );
						ShowWindow( hWndDBPASSWORD, SW_SHOW );
						ShowWindow( hWndACCESSFILE, SW_SHOW );
						ShowWindow( hWndPrivate, SW_SHOW );
						SetWindowText( hWndCONFIG, "Status" );
						for(int i = 0; i< 30;i++)
						{
							ShowWindow( hWndSettings[i], SW_SHOW );
						}
						ShowWindow( g_hWndConsole, SW_HIDE );
						g_fConfig = TRUE;
					}
					else
					{
						MoveWindow( hWnd,0 , 0 , CONSOLE_WIDTH_NORM ,CONSOLE_HEIGHT_NORM , TRUE );						
						ShowWindow( hWndDBIP, SW_HIDE );
						ShowWindow( hWndDBNAME, SW_HIDE );
						ShowWindow( hWndDBUSER, SW_HIDE );
						ShowWindow( hWndDBPASSWORD, SW_HIDE );
						ShowWindow( hWndACCESSFILE, SW_HIDE );
						ShowWindow( hWndPrivate, SW_HIDE );
						SetWindowText( hWndCONFIG, "Settings" );
						for(int i = 0; i< 30;i++)
						{
							ShowWindow( hWndSettings[i], SW_HIDE );
						}
						ShowWindow( g_hWndConsole, SW_SHOW );
						g_fConfig = FALSE;
					}

					break;
				}

				case IDC_ACCESS_DB:
				{
					// Disable Dialog boxes
					EnableWindow( hWndDBIP, FALSE );
					EnableWindow( hWndDBNAME, FALSE );
					EnableWindow( hWndDBUSER, FALSE );
					EnableWindow( hWndDBPASSWORD, FALSE );
					EnableWindow( hWndACCESSFILE, TRUE );

					CheckDlgButton( hWnd, IDC_ACCESS_DB, 1 );
					CheckDlgButton( hWnd, IDC_MSSQL_DB, 0 );
					CheckDlgButton( hWnd, IDC_MYSQL_DB, 0 );

					SetDlgItemText( hWnd, IDC_DBTYPE, "1" );

					UpdateHelpTitle( "Microsoft Access:\r\n");
					UpdateHelp( "A relational database management system supported by the server. Requires the use of a Microsoft Access (.mdb) database file.\r\n");

					break;
				}

				case IDC_MSSQL_DB:
				{
					// Enable Dialog boxes
					EnableWindow( hWndDBIP, TRUE );
					EnableWindow( hWndDBNAME, TRUE );
					EnableWindow( hWndDBUSER, TRUE );
					EnableWindow( hWndDBPASSWORD, TRUE );
					EnableWindow( hWndACCESSFILE, FALSE );
					
					CheckDlgButton( hWnd, IDC_ACCESS_DB, 0 );
					CheckDlgButton( hWnd, IDC_MSSQL_DB, 1 );
					CheckDlgButton( hWnd, IDC_MYSQL_DB, 0 );

					SetDlgItemText( hWnd, IDC_DBTYPE, "2" );
					
					UpdateHelpTitle( "MS SQL:\r\n");
					UpdateHelp( "A relational database management system supported by the server. Requires access to an MS SQL database.\r\n");
						
					break;
				}

				case IDC_MYSQL_DB:
				{
					// Enable Dialog boxes
					EnableWindow( hWndDBIP, TRUE );
					EnableWindow( hWndDBNAME, TRUE );
					EnableWindow( hWndDBUSER, TRUE );
					EnableWindow( hWndDBPASSWORD, TRUE );
					EnableWindow( hWndACCESSFILE, FALSE );
					
					CheckDlgButton( hWnd, IDC_ACCESS_DB, 0 );
					CheckDlgButton( hWnd, IDC_MSSQL_DB, 0 );
					CheckDlgButton( hWnd, IDC_MYSQL_DB, 1 );

					SetDlgItemText( hWnd, IDC_DBTYPE, "3" );
					
					UpdateHelpTitle( "MySQL:\r\n");
					UpdateHelp( "A relational database management system supported by the server. Requires access to a MySQL database.\r\n");

					break;
				}

				case IDC_BTN_UPDATE:
				{
					char szTemp[5];
					char szDBType[2];
					char szDBTemp[20];

					HKEY hKey;
					RegCreateKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\UAS", NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
					
					GetDlgItemText( hWnd, IDC_LOCALIP, g_szLocalIP, sizeof( g_szLocalIP ) );
					RegSetValueEx( hKey, "LocalIP", NULL, REG_SZ, (BYTE*)g_szLocalIP, lstrlen( g_szLocalIP ) );
					
					GetDlgItemText( hWnd, IDC_CHARPORT, szTemp, sizeof( szTemp ) );
					RegSetValueEx( hKey, "CharPort", NULL, REG_SZ, (BYTE*)szTemp, lstrlen( szTemp ) );

					GetDlgItemText( hWnd, IDC_WORLDPORT, szTemp, sizeof( szTemp ) );
					RegSetValueEx( hKey, "WorldPort", NULL, REG_SZ, (BYTE*)szTemp, lstrlen( szTemp ) );

					// Database Settings //////////////////////////////////////////////////////////////
					GetDlgItemText( hWnd, IDC_DBTYPE, szDBType, sizeof( szDBType ) );
					RegSetValueEx( hKey, "DBTYPE", NULL, REG_SZ, (BYTE*)szDBType, lstrlen( szDBType ) );

					GetDlgItemText( hWnd, IDC_DBIP, szDBTemp, sizeof( szDBTemp ) );
					RegSetValueEx( hKey, "DBIP", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
					
					GetDlgItemText( hWnd, IDC_DBNAME, szDBTemp, sizeof( szDBTemp ) );
					RegSetValueEx( hKey, "DBNAME", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
					
					GetDlgItemText( hWnd, IDC_DBUSER, szDBTemp, sizeof( szDBTemp ) );
					RegSetValueEx( hKey, "DBUSER", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
					
					GetDlgItemText( hWnd, IDC_DBPASSWORD, szDBTemp, sizeof( szDBTemp ) );
					RegSetValueEx( hKey, "DBPASSWORD", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
					
					char szAccessTemp[MAX_PATH+20];
					GetDlgItemText( hWnd, IDC_ACCESS_FILE, szAccessTemp, sizeof( szAccessTemp ) );
					RegSetValueEx( hKey, "ACCESSMDB", NULL, REG_SZ, (BYTE*)szAccessTemp, lstrlen( szAccessTemp ) );

					///////////////////////////////////////////////////////////////////////////////////
					// Status Server Settings
					char szHostTemp[64];

					int nState;
					nState = IsDlgButtonChecked( hWnd, IDC_SERVER);
					sprintf(szDBTemp,"%d",nState);
					RegSetValueEx( hKey, "Private", NULL, REG_SZ, (BYTE*)szDBTemp, sizeof(szDBTemp) );

					GetDlgItemText( hWnd, IDC_ED_HOST, szHostTemp, sizeof( szHostTemp ) );
					RegSetValueEx( hKey, "Status Host", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
								
					GetDlgItemText( hWnd, IDC_ED_PORT, szDBTemp, sizeof( szDBTemp ) );
					RegSetValueEx( hKey, "HTTP Port", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
											
					GetDlgItemText( hWnd, IDC_ED_ID, szDBTemp, sizeof( szDBTemp ) );
					RegSetValueEx( hKey, "Server ID", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
														
					GetDlgItemText( hWnd, IDC_ED_SNAME, szHostTemp, sizeof( szHostTemp ) );
					RegSetValueEx( hKey, "Server Name", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
					memcpy(cMasterServer::m_szServerName,szHostTemp,sizeof(szHostTemp));
																	
					GetDlgItemText( hWnd, IDC_ED_SERIAL, szHostTemp, sizeof( szHostTemp ) );
					RegSetValueEx( hKey, "Server Serial", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
																	
					GetDlgItemText( hWnd, IDC_ED_KEY, szHostTemp, sizeof( szHostTemp ) );
					RegSetValueEx( hKey, "Server Key", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
																	
					GetDlgItemText( hWnd, IDC_ED_URL, szHostTemp, sizeof( szHostTemp ) );
					RegSetValueEx( hKey, "HTTP Path", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
																				
					GetDlgItemText( hWnd, IDC_ED_CLIENT, szHostTemp, sizeof( szHostTemp ) );
					RegSetValueEx( hKey, "Client Support", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
																				
					GetDlgItemText( hWnd, IDC_MAX_USERS, szHostTemp, sizeof( szHostTemp ) );
					RegSetValueEx( hKey, "Max Clients", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
					/////////////////////////////////////////////////////////////////////////////////////
					
					RegCloseKey( hKey );
					break;
				}
				default:
					break;	// case default
			}
			break;	// case WM_COMMAND
		}

		case MYWM_NOTIFYICON:
		{
			switch (lParam)
			{
				case WM_LBUTTONDOWN:
				{
					ShowWindow( hWnd, SW_RESTORE );
					SetForegroundWindow( hWnd );
					SendMessage( hWnd, WM_ACTIVATEAPP, ( WPARAM )TRUE, ( LPARAM )NULL );
					break;
				}

				case WM_RBUTTONUP:
				{
					HMENU hTrayMenu;
					HMENU hMenu;
					POINT point;

					SetForegroundWindow( hWnd );

					hTrayMenu = LoadMenu( g_hInstance, MAKEINTRESOURCE( ID_SYSTRAY ) );
					hMenu = GetSubMenu( hTrayMenu, 0 );
					GetCursorPos( &point );

					TrackPopupMenu( hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL );
					DestroyMenu( hMenu );
					DestroyMenu( hTrayMenu );

					PostMessage( hWnd, WM_NULL, 0, 0 ); 

					break;
				}
			}
			break;
		}
		case WM_CLOSE:
		{
			char szTemp[5];
			
			HKEY hKey;
			RegCreateKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\UAS", NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
			
			GetDlgItemText( hWnd, IDC_LOCALIP, g_szLocalIP, sizeof( g_szLocalIP ) );
			RegSetValueEx( hKey, "LocalIP", NULL, REG_SZ, (BYTE*)g_szLocalIP, lstrlen( g_szLocalIP ) );
			
			GetDlgItemText( hWnd, IDC_CHARPORT, szTemp, sizeof( szTemp ) );
			RegSetValueEx( hKey, "CharPort", NULL, REG_SZ, (BYTE*)szTemp, lstrlen( szTemp ) );

			GetDlgItemText( hWnd, IDC_WORLDPORT, szTemp, sizeof( szTemp ) );
			RegSetValueEx( hKey, "WorldPort", NULL, REG_SZ, (BYTE*)szTemp, lstrlen( szTemp ) );

			// Database Settings //////////////////////////////////////////////////////////////
			char szDBType[2];
			char szDBTemp[20];

			GetDlgItemText( hWnd, IDC_DBTYPE, szDBType, sizeof( szDBType ) );
			RegSetValueEx( hKey, "DBTYPE", NULL, REG_SZ, (BYTE*)szDBType, lstrlen( szDBType ) );

			GetDlgItemText( hWnd, IDC_DBIP, szDBTemp, sizeof( szDBTemp ) );
			RegSetValueEx( hKey, "DBIP", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
			
			GetDlgItemText( hWnd, IDC_DBNAME, szDBTemp, sizeof( szDBTemp ) );
			RegSetValueEx( hKey, "DBNAME", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
			
			GetDlgItemText( hWnd, IDC_DBUSER, szDBTemp, sizeof( szDBTemp ) );
			RegSetValueEx( hKey, "DBUSER", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
			
			GetDlgItemText( hWnd, IDC_DBPASSWORD, szDBTemp, sizeof( szDBTemp ) );
			RegSetValueEx( hKey, "DBPASSWORD", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
			
			char szAccessTemp[MAX_PATH+20];
			GetDlgItemText( hWnd, IDC_ACCESS_FILE, szAccessTemp, sizeof( szAccessTemp ) );
			RegSetValueEx( hKey, "ACCESSMDB", NULL, REG_SZ, (BYTE*)szAccessTemp, lstrlen( szAccessTemp ) );
			///////////////////////////////////////////////////////////////////////////////////
			// Status Server Settings
			char szHostTemp[64];

			int nState;
			nState = IsDlgButtonChecked( hWnd, IDC_SERVER);
			sprintf(szDBTemp,"%d",nState);
			RegSetValueEx( hKey, "Private", NULL, REG_SZ, (BYTE*)szDBTemp, sizeof(szDBTemp) );

			GetDlgItemText( hWnd, IDC_ED_HOST, szHostTemp, sizeof( szHostTemp ) );
			RegSetValueEx( hKey, "Status Host", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
						
			GetDlgItemText( hWnd, IDC_ED_PORT, szDBTemp, sizeof( szDBTemp ) );
			RegSetValueEx( hKey, "HTTP Port", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
									
			GetDlgItemText( hWnd, IDC_ED_ID, szDBTemp, sizeof( szDBTemp ) );
			RegSetValueEx( hKey, "Server ID", NULL, REG_SZ, (BYTE*)szDBTemp, lstrlen( szDBTemp ) );
												
			GetDlgItemText( hWnd, IDC_ED_SNAME, szHostTemp, sizeof( szHostTemp ) );
			RegSetValueEx( hKey, "Server Name", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
															
			GetDlgItemText( hWnd, IDC_ED_SERIAL, szHostTemp, sizeof( szHostTemp ) );
			RegSetValueEx( hKey, "Server Serial", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
															
			GetDlgItemText( hWnd, IDC_ED_KEY, szHostTemp, sizeof( szHostTemp ) );
			RegSetValueEx( hKey, "Server Key", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
															
			GetDlgItemText( hWnd, IDC_ED_URL, szHostTemp, sizeof( szHostTemp ) );
			RegSetValueEx( hKey, "HTTP Path", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
																		
			GetDlgItemText( hWnd, IDC_ED_CLIENT, szHostTemp, sizeof( szHostTemp ) );
			RegSetValueEx( hKey, "Client Support", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
																				
			GetDlgItemText( hWnd, IDC_MAX_USERS, szHostTemp, sizeof( szHostTemp ) );
			RegSetValueEx( hKey, "Max Clients", NULL, REG_SZ, (BYTE*)szHostTemp, lstrlen( szHostTemp ) );
			/////////////////////////////////////////////////////////////////////////////////////
			
			RegCloseKey( hKey );

			if ( g_fStarted )
			{
				fExit = TRUE;
				SendMessage( hWnd, WM_COMMAND, IDB_START, TRUE );
			}
			else
			{
				WSACleanup();
				SystrayDelete( hWnd );
				DestroyWindow( hWnd );
				PostQuitMessage( 0 );
			}

			break;	// case WM_CLOSE
		}

		default:
			if( ( msg == s_uTaskbarRestart ) && ( fMinimized ) )
			{
				SystrayDelete( hWnd );
				SystrayAdd( hWnd );
				break;
			}
	}
	return FALSE;
}

WORD	g_wAvatarTexturesList[Race][Gender][TexType][TexIndex];
WORD	g_wAvatarTexturesBaldList[Race][Gender][TexIndex];

void LoadAvatarTextureList()
{
	char *pbuf;
	DWORD dwSize;
	HRSRC hRSRC;
	HGLOBAL hGlobal;

	hRSRC = FindResource( g_hInstance, MAKEINTRESOURCE( IDR_BINDATA ), "BINARY" );
	if( hRSRC == NULL )
		UpdateConsole( " FindResource", GetLastError( ) );

	dwSize = SizeofResource( g_hInstance, hRSRC );
	hGlobal = LoadResource( g_hInstance, hRSRC );
	if( hGlobal == NULL )
		UpdateConsole( " LoadResource", GetLastError( ) );

	pbuf = (char*)LockResource( hGlobal );
	if( pbuf == NULL )
		UpdateConsole( " LockResource", GetLastError( ) );

	CopyMemory(g_wAvatarTexturesList, pbuf, sizeof( g_wAvatarTexturesList ) );
	CopyMemory(g_wAvatarTexturesBaldList, pbuf + sizeof( g_wAvatarTexturesList ), sizeof( g_wAvatarTexturesBaldList ) );
}

/**
 *	The entry point of the application.
 *
 *	@param hInstance - Handle to the current instance of the application.
 *	@param hPrevInstance - Handle to the previous instance of the application (always NULL).
 *	@param lpCmdLine - Pointer to a null-terminated string specifying the command line for the application, excluding the program name.
 *	@param iCmdShow - Specifies how the window is to be shown.
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow )
{
	g_hInstance = hInstance;
	
	InitCommonControls( );
	cDatabase::InitializeSQLEnvironment();
	LoadAvatarTextureList();
	
	g_hWndMain	= CreateDialog( hInstance, MAKEINTRESOURCE( IDD_MAIN ), NULL, MainWindowProc );

	if ( !g_hWndMain )
		return -1;

	g_fStarted			= FALSE;

	ShowWindow( g_hWndMain, iCmdShow );

	MSG msg;
	while ( GetMessage( &msg, NULL, 0, 0 ) )
	{
		if( IsDialogMessage( g_hWndMain, &msg ) )
			continue;

		TranslateMessage( &msg );
		DispatchMessage( &msg );
    }

	WSACleanup( );

	cDatabase::FreeSQLEnvironment();
	
	return msg.wParam;
}