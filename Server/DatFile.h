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

/* Special thanks to David Simpson for his work on the cell.dat and portal.dat extraction code */

/**
 *	@file DatFile.h
 */

#ifndef __DATFILE_H
#define __DATFILE_H

#pragma warning(disable:4786)	//warning: identifier was truncated to '255' characters in the browser information

#include <winsock2.h>
#include <list>
#include <algorithm>

#include "Avatar.h"

#define CELLSECSIZE		64
#define PORTALSECSIZE	256
#define NUMFILELOC		0x03F
#define ROOTDIRPTRLOC	0x148

class cPortalDat
{

public:
	
	static void		LoadStartingInfo	( cAvatar *pcAvatar );
	static void		LoadItemModel		( cObject *pcObject, DWORD dwModelID, DWORD dwColorID = 0, double dblColorValue = -1 );
	static int		CalcPalette			( int numPalettes, double palValue = -1 );

	/**
	 *	Loads the portal.dat information for the specified file.
	 *
	 *	Calls FetchPortalFilePos to find a file's position.
	 *	Calls FetchPortalFile to fetch a file from its found position.
	 *
	 *	@param &buf - The address of the buffer that should receive the file information.
	 *	@param file - The file to be loaded from the portal.dat.
	 */
	static inline void Load_PortalDat( UCHAR* &buf, char file[9] )
	{
		FILE	*inFile;
		int		read;
		UINT	rootDirPtr;
		UINT	filePos, len;
		UINT	id;

		inFile = fopen("portal.dat", "rb");
		if (inFile == NULL)
		{
			UpdateConsole(" Portal.dat: Open failed.\r\n");
		//	return -1;
		}

		read = fseek(inFile, ROOTDIRPTRLOC, SEEK_SET);
		if (read != 0)
		{
			UpdateConsole(" Portal.dat: Read error.\r\n");
			fclose(inFile);
		//	return -1;
		}

		read = fread(&rootDirPtr, sizeof(UINT), 1, inFile);
		if (read != 1)
		{
			UpdateConsole(" Portal.dat: End of file reached.\r\n");
			fclose(inFile);
		//	return 0;
		}

		id = strtol(file, NULL, 16);

		if (!FetchPortalFilePos(inFile, rootDirPtr, id, &filePos, &len))
		{
			UpdateConsole(" Portal.dat: File not found.\r\n");
		//	return -1;
		}

		buf = (UCHAR *)malloc(len);
		if (!FetchPortalFile(inFile, filePos, len, buf))
		{
			free(buf);
			fclose(inFile);
		//	return -1;
		}

		//free(buf);
		fclose(inFile);
	}

private:

	/**
	 *	Finds the location of a file in the portal.dat.
	 *
	 *	@param &inFile - The address of the file/directory to search (the portal.dat).
	 *	@param disPos - The beginning position of the file pointer (in BYTEs).
	 *	@param id - The file to be loaded from the portal.dat.
	 *	@param *filePos - A pointer to the variable that should receive the file's position.
	 *	@param *len - A pointer to the variable that should receive the file's length.
	 */
	static inline int FetchPortalFilePos(FILE *inFile, UINT dirPos, UINT id, UINT *filePos, UINT *len)
	{

		UINT dir[PORTALSECSIZE];
		UINT i;
		UINT numFiles;
		int  read;

		while (1)
		{
			if (dirPos == 0)
			{
				UpdateConsole(" Portal.dat: NULL directory entry found.\r\n");
				return 0;
			}

			read = fseek(inFile, dirPos, SEEK_SET);
			if (read != 0)
			{
				UpdateConsole(" Portal.dat: Sector is beyond end of file.\r\n");
				return 0;
			}

			read = fread(dir, sizeof(UINT), PORTALSECSIZE, inFile);
			if (read != PORTALSECSIZE)
			{
				UpdateConsole(" Portal.dat: Sector doesn't contain enough words.\r\n");
				return 0;
			}

			numFiles = dir[NUMFILELOC];
			if (numFiles >= NUMFILELOC)
			{
				UpdateConsole(" Portal.dat: Number of files exceeds directory entries.\r\n");
				return 0;
			}

			i = 0;
			while ((i < numFiles) && (id > dir[i * 3 + NUMFILELOC + 1]))
			{
				i++;
			}
			if (i < numFiles)
			{
				if (id == dir[i * 3 + NUMFILELOC + 1])
				{
					*filePos = dir[i * 3 + NUMFILELOC + 2];
					*len = dir[i * 3 + NUMFILELOC + 3];
					return 1;
				}
			}

			if (dir[1] == 0)
			{
				filePos = 0;
				len = 0;
				return 0;
			}
			dirPos = dir[i + 1];
		}

		return 0;
	}

	/**
	 *	Fetches a file from the portal.dat.
	 *
	 *	@param *inFile - A pointer to the file/directory to search (the portal.dat).
	 *	@param filePos - The variable that should receive the file's position.
	 *	@param len - The variable that should receive the file's length.
	 *	*@param &buf - A pointer to the buffer that should receive the file information.
	 */
	static inline int FetchPortalFile(FILE *inFile, UINT filePos, UINT len, UCHAR *buf)
	{
		int  read, doChain;
		UINT sec[PORTALSECSIZE];

		if (filePos == 0)
		{
			UpdateConsole(" Portal.dat: Null file pointer found.\r\n");
			return 0;
		}

		doChain = 1;
		while (doChain)
		{

			read = fseek(inFile, filePos, SEEK_SET);
			if (read != 0)
			{
				UpdateConsole(" Portal.dat: Seek failed.\r\n");
				return 0;
			}
			read = fread(sec, sizeof(UINT), PORTALSECSIZE, inFile);
			if (read != PORTALSECSIZE)
			{
				UpdateConsole(" Portal.dat: Sector doesn't contain enough words.\r\n");
				return 0;
			}
    
			filePos = sec[0] & 0x7FFFFFFF;

			if (len > (PORTALSECSIZE  - 1) * sizeof(UINT))
			{
				memcpy(buf, &sec[1], (PORTALSECSIZE - 1) * sizeof(UINT));
				buf += (PORTALSECSIZE - 1) * sizeof(UINT);
				len -= (PORTALSECSIZE - 1) * sizeof(UINT);
			} else {
				memcpy(buf, &sec[1], len);
				len = 0;
			}

			if (filePos == 0)
				doChain = 0;
		}

		return 1;
	}
};

#endif	// #ifndef __DATFILE_H