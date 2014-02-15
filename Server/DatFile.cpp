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
 *	@file DatFile.cpp
 *	Handles the general operations relating to the database.
 *
 *	Includes the functionality to create accounts, create, delete, and load avatars,
 *	and to initalize the Global Unique Identifier (GUID) values.
 */

#include "Avatar.h"
#include "DatFile.h"

/**
 *	Navigates the Character Creation Information (0x0E000002) file in the 
 *	portal.dat.  This file contains the starting locations, race- and
 *	gender-specific clothing and color selections, and the male and female
 *	palette, texture, and model information.  This model information includes 
 *	the female/male models, skin shade, hair color, hair shade, hair style, 
 *	eye color, forehead style, chin style, and nose style.
 *
 *	@param *pcAvatar - A pointer to the avatar whose information should be loaded or updated.
 *
 *	Author: eLeM
 */
void cPortalDat::LoadStartingInfo( cAvatar *pcAvatar )
{
	DWORD	dwRace				= pcAvatar->m_wRace;
	DWORD	dwSex				= pcAvatar->m_wGender;
	WORD	wClass				= pcAvatar->m_wClass;
	double	dblSkinShade		= pcAvatar->m_dblSkinShade;
	DWORD	dwHairColor			= pcAvatar->m_wHairColor;
	double	dblHairShade		= pcAvatar->m_dblHairShade;
	DWORD	dwHairStyle			= pcAvatar->m_wHairStyle;
	DWORD	dwEyeColor			= pcAvatar->m_wEyeColor;
	DWORD	dwForeheadTexture	= pcAvatar->m_wHead;
	DWORD	dwNoseTexture		= pcAvatar->m_wNose;
	DWORD	dwChinTexture		= pcAvatar->m_wChin;

	/* Inputs */
	//dwRace			- Determines the proper race to reference
	//dwSex				- Determines the proper gender to reference
	//dwClass			- Determines the proper class/profession to reference
	//dblSkinShade		- Determines the palette selected from the skin palette list
	//dwHairColor		- Determines the palette list selected from the list of hair color palette lists
	//dblHairShade		- Determines the palette selected from the selected hair color palette list
	//dwHairStyle		- Determines the texture selected from the list of hair style textures
	//dwEyeColor		- Determines the palette selected from the list of eye color palettes
	//dwForeheadTexture	- Determines the textures selected from the list of forehead textures
	//dwNoseTexture		- Determines the texture selected from the list of nose textures
	//dwChinTexture		- Determines the texture selected from the list of chin textures
	
	//Model variables
	//Used to populate any character palette, texture, and model information fetched by the function
	pcAvatar->m_wBasicPaletteVector	= 0;
	pcAvatar->m_wBasicTextureVector	= 0;
	pcAvatar->m_wBasicModelVector	= 0;

	char	portalFile[9];
	sprintf(portalFile,"%08x",0x0E000002 );	//Character Creation Objects

	//* Character Creation Objects (0x0E000002) Algorithm *//
	//This algorithm fetches starting character information from the portal.dat,
	//The data can be used to populate the character-based palette, texture, and model informtion.

//	char	szData[255];
	UCHAR	*buf;

	//DWORD	fileNum;
	DWORD	startingRegionsCount;

	//Load data from the portal.dat file
	cPortalDat::Load_PortalDat(buf, portalFile);
	int readIndex = 0;
	
	//memcpy(&fileNum,buf,4);
	readIndex += 4;	//the first 4 bytes list the index for which we searched

	readIndex += 4*8;	//Eight UI Text DWORDs

	memcpy(&startingRegionsCount,buf + readIndex,4);	readIndex += 4;

	for (int xIndex = 0; xIndex < startingRegionsCount; ++xIndex)
	{
		WORD	regionNameLength;
		char	szRegionName[40];
		DWORD	locationCount;
		
		memcpy(&regionNameLength,buf + readIndex,2);			readIndex += 2;
		memcpy(&szRegionName,buf + readIndex,regionNameLength);	readIndex += regionNameLength;

		while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding

		memcpy(&locationCount,buf + readIndex,4);		readIndex += 4;

		for (int yIndex = 0; yIndex < locationCount; yIndex++)
		{
//			DWORD	landblock;
//			DWORD	positionX, positionY, positionZ;
//			DWORD	oritentationW,oritentationX,oritentationY,oritentationZ;
			
			readIndex += 4*8;	//region landblock,x,y,z,w,x,y,z information
		}
	}

	DWORD	raceCount;

	memcpy(&raceCount,buf + readIndex,4);	readIndex += 4;

	for (int rIndex = 0; rIndex < raceCount; rIndex++)
	{
		WORD	raceNameLength;
		char	szRaceName[40];
//		DWORD	raceUIGraphic
//		DWORD	raceUIText;
		DWORD	genderCount;
		
		memcpy(&raceNameLength,buf + readIndex,2);			readIndex += 2;
		memcpy(&szRaceName,buf + readIndex,raceNameLength);	readIndex += raceNameLength;
		
		//Determine if this is the avatar's race
		if (rIndex == dwRace)
		{
			memcpy(pcAvatar->m_strRaceName,szRaceName,raceNameLength);
			pcAvatar->m_strRaceName[raceNameLength] = '\0';
		}

		while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding

		readIndex += 4;		//raceUIGraphic
		readIndex += 4;		//unknown
		readIndex += 4;		//raceUIText
		readIndex += 4;		//unknown

		readIndex += 4*8;	//Eight unknown DWORDs
		
		memcpy(&genderCount,buf + readIndex,4);	readIndex += 4;

		for (int gIndex = 0; gIndex < genderCount; gIndex++)
		{
			WORD	genderNameLength;
			char	szGenderName[40];
			DWORD	genderModel;
			BYTE	hPaletteCount,hTextureCount,hModelCount;
			DWORD	unknownCount;
			DWORD	namePrefixCount;
			DWORD	classCount;
			DWORD	unknown1;

			memcpy(&genderNameLength,buf + readIndex,2);			readIndex += 2;
			memcpy(&szGenderName,buf + readIndex,genderNameLength);	readIndex += genderNameLength;

			//Determine if this is the avatar's gender
			if (rIndex == dwRace && gIndex == dwSex)
			{
				memcpy(pcAvatar->m_strSexName,szGenderName,genderNameLength);
				pcAvatar->m_strSexName[genderNameLength] = '\0';
			}

			while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding

			memcpy(&genderModel,buf + readIndex,4);		readIndex += 4;
/*
			sprintf(szData,"%04x\r\n",genderModel);
			UpdateConsole(szData);
*/
			readIndex += 4*3;	//Three unknown DWORDs

			readIndex += 1;		//BYTE 11
			memcpy(&hPaletteCount,buf + readIndex,1);	readIndex += 1;	//Palette Count
			memcpy(&hTextureCount,buf + readIndex,1);	readIndex += 1;	//Texture Count
			memcpy(&hModelCount,buf + readIndex,1);		readIndex += 1;	//Model Count

			for (int pIndex = 0; pIndex < hPaletteCount; ++pIndex)	//Palettes
			{
				WORD	newPalette;
				BYTE	palOffset;
				UCHAR	palLength;

				memcpy(&newPalette,buf+readIndex,2);	readIndex += 2;
				memcpy(&palOffset,buf+readIndex,1);		readIndex += 1;
				memcpy(&palLength,buf+readIndex,1);		readIndex += 1;

				if (rIndex == dwRace && gIndex == dwSex)
				{
					pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_wNewPalette = WORD(newPalette);
					pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucOffset = BYTE(palOffset);
					pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucLength = BYTE(palLength);
					pcAvatar->m_wBasicPaletteVector++;
				}
			}
			for (int tIndex = 0; tIndex < hTextureCount; ++tIndex)	//Textures
			{
				BYTE	modelIndex;
				WORD	textureMask;
				WORD	texture;

				memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
				memcpy(&textureMask,buf+readIndex,2);	readIndex += 2;
				memcpy(&texture,buf+readIndex,2);		readIndex += 2;

				if (rIndex == dwRace && gIndex == dwSex)
				{
					pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_bModelIndex = BYTE(modelIndex);
					pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wOldTexture = WORD(textureMask);
					pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wNewTexture = WORD(texture);
					pcAvatar->m_wBasicTextureVector++;
				}
			}
			for (int mIndex = 0; mIndex < hModelCount; ++mIndex)	//Models
			{
				BYTE	modelIndex;
				WORD	model;

				memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
				memcpy(&model,buf+readIndex,2);			readIndex += 2;

				if (rIndex == dwRace && gIndex == dwSex)
				{
					pcAvatar->m_BasicVectorMod[pcAvatar->m_wBasicModelVector].m_bModelIndex =  BYTE(modelIndex);
					pcAvatar->m_BasicVectorMod[pcAvatar->m_wBasicModelVector].m_wNewModel = WORD(model);
					pcAvatar->m_wBasicModelVector++;
				}
			}
			if (hTextureCount > 0 && hModelCount == 0)
				readIndex += 3;		//Every texture has corresponding model information

			readIndex += 1;		//BYTE -- constant zero

			readIndex += 4*2;	//Two unknown DWORDs

			memcpy(&namePrefixCount,buf + readIndex,4);	readIndex += 4;
			
			//Loop through name prefixes (i.e., Gharu'ndim "-al" and "ibn")
			for (int zIndex = 0; zIndex < namePrefixCount; ++zIndex)
			{
				WORD	namePrefixLength;
				char	szNamePrefix[40];

				memcpy(&namePrefixLength,buf + readIndex,2);			readIndex += 2;
				memcpy(&szNamePrefix,buf + readIndex,namePrefixLength);	readIndex += namePrefixLength;

				while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding
			}

			readIndex += 4*5;	//Five unknown DWORDs

			memcpy(&unknownCount,buf + readIndex,4);	readIndex += 4;

			for (zIndex = 0; zIndex < unknownCount; ++zIndex)
			{
				readIndex += 4*3;	//Three unknown DWORDs
			}

			memcpy(&classCount,buf + readIndex,4);		readIndex += 4;
/*
			sprintf(szData,"%04x\r\n",classCount);
			UpdateConsole(szData);
*/
			for (int cIndex = 0; cIndex < classCount; ++cIndex)
			{

				WORD	classNameLength;
				char	szClassName[40];
				DWORD	classUIGraphic;
				DWORD	classUIText;

				DWORD	valueUnknown1;
				DWORD	valueStrength;
				DWORD	valueEndurance;
				DWORD	valueCoordination;
				DWORD	valueQuickness;
				DWORD	valueFocus;
				DWORD	valueSelf;

				DWORD	countTrain;
				DWORD	countSpecialize;
				DWORD	countUnknown;
				DWORD	skillNum;

				memcpy(&classNameLength,buf + readIndex,2);				readIndex += 2;
				memcpy(&szClassName,buf + readIndex,classNameLength);	readIndex += classNameLength;
					
				//Determine if this is the avatar's class
				if (rIndex == dwRace && gIndex == dwSex)
				{
					if (cIndex == wClass)
					{
						memcpy(pcAvatar->m_strClassName,szClassName,classNameLength);
						pcAvatar->m_strClassName[classNameLength] = '\0';
					}
				}

				while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding

				memcpy(&classUIGraphic,buf + readIndex,4);		readIndex += 4;
				memcpy(&classUIText,buf + readIndex,4);			readIndex += 4;

				memcpy(&valueUnknown1,buf + readIndex,4);		readIndex += 4;
				memcpy(&valueStrength,buf + readIndex,4);		readIndex += 4;
				memcpy(&valueEndurance,buf + readIndex,4);		readIndex += 4;
				memcpy(&valueCoordination,buf + readIndex,4);	readIndex += 4;
				memcpy(&valueQuickness,buf + readIndex,4);		readIndex += 4;
				memcpy(&valueFocus,buf + readIndex,4);			readIndex += 4;
				memcpy(&valueSelf,buf + readIndex,4);			readIndex += 4;

				memcpy(&countTrain,buf + readIndex,4);		readIndex += 4;

				for (int skillTIndex = 0; skillTIndex < countTrain; ++skillTIndex)
				{
					memcpy(&skillNum,buf + readIndex,4);	readIndex += 4;
				}

				memcpy(&countSpecialize,buf + readIndex,4);readIndex += 4;

				for (int skillSIndex = 0; skillSIndex < countSpecialize; ++skillSIndex)
				{
					memcpy(&skillNum,buf + readIndex,4);	readIndex += 4;
				}

				memcpy(&countUnknown,buf + readIndex,4);	readIndex += 4;
			}

			memcpy(&unknown1,buf + readIndex,4);			readIndex += 4;		//Unknown DWORD

			DWORD	humanPalette;
			DWORD	skinPaletteList;
			DWORD	hairColorPaletteListCount;
			DWORD	hairColorPaletteList;
			DWORD	hairStyleCount;
			DWORD	eyeColorPaletteCount;
			DWORD	foreheadTextureCount;
			DWORD	noseTextureCount;
			DWORD	chinTextureCount;

//			DWORD	paletteFileNum;
			DWORD	paletteCount;
			int		palReadIndex;

			char	szPalFile[9];
			UCHAR	*palBuf;
			int intPalette;

			memcpy(&humanPalette,buf + readIndex,4);		readIndex += 4;	//Human palette (0x0400007E)

			/* Determine skin color information */
			//Each skin color has a single CLUT list associated with it
			//If a palette from the list is used, the palette is determined via the dblSkinShade input value
			memcpy(&skinPaletteList,buf + readIndex,4);	readIndex += 4;	//Skin CLUT List (0x0F000000 value)
/*			
			sprintf(szData,"Palette ID: %08x\r\n",skinPaletteList);
			UpdateConsole(szData);
*/			
			sprintf(szPalFile,"%08x",skinPaletteList);

			//Load palette data from the portal.dat file
			cPortalDat::Load_PortalDat(palBuf, szPalFile);

			palReadIndex = 0;
			palReadIndex += 4;	//the first 4 bytes list the index for which we searched

			memcpy(&paletteCount,palBuf+palReadIndex,4);
			palReadIndex += 4;
/*
			sprintf(szData,"Palette Count: %08x\r\n",paletteCount);
			UpdateConsole(szData);
*/
			//Calculate the palette to be used
			intPalette = CalcPalette(paletteCount,dblSkinShade);

			//Loop through the specific palettes (0x04000000 values)
			for (zIndex = 0; zIndex < paletteCount; ++zIndex)
			{
				DWORD palette;
				memcpy(&palette,palBuf+palReadIndex,4);
				palReadIndex += 4;
/*
				sprintf(szData,"Palette: %08x\r\n",palette);
				UpdateConsole(szData);
*/			
				if (zIndex == intPalette)
				{
					if (rIndex == dwRace && gIndex == dwSex)
					{
						pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_wNewPalette = WORD(palette | 0x04000000);
						pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucOffset = BYTE(0x00);
						pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucLength = BYTE(0x18);
						pcAvatar->m_wBasicPaletteVector++;
					}
				}
			}
			free(palBuf);

			/* Determine hair color information */
			//Each hair color has a list of palette lists associated with it
			//If a palette list is used, the list is determined via the dwHairStyle input value
			//The palette choosen from the palette list is determined via the dblHairShade input value
			memcpy(&hairColorPaletteListCount,buf + readIndex,4);	readIndex += 4;
			
			for (int wIndex = 0; wIndex < hairColorPaletteListCount; ++wIndex)
			{
				memcpy(&hairColorPaletteList,buf + readIndex,4);	readIndex += 4;	//Hair color CLUT List (0x0F000000 value)
/*				
				sprintf(szData,"Palette ID: %08x\r\n",hairColorPaletteList);
				UpdateConsole(szData);
*/				
				if (wIndex == dwHairColor)
				{
					sprintf(szPalFile,"%08x",hairColorPaletteList);

					//Load palette data from the portal.dat file
					cPortalDat::Load_PortalDat(palBuf, szPalFile);

					palReadIndex = 0;
					palReadIndex += 4;	//the first 4 bytes list the index for which we searched

					memcpy(&paletteCount,palBuf+palReadIndex,4);
					palReadIndex += 4;
/*
					sprintf(szData,"Palette Count: %08x\r\n",paletteCount);
					UpdateConsole(szData);
*/
					//Calculate the palette to be used
					intPalette = CalcPalette(paletteCount,dblHairShade);

					//Loop through the specific palettes (0x04000000 values)
					for (zIndex = 0; zIndex < paletteCount; ++zIndex)
					{
						DWORD palette;
						memcpy(&palette,palBuf+palReadIndex,4);
						palReadIndex += 4;
	/*
						sprintf(szData,"Palette: %08x\r\n",palette);
						UpdateConsole(szData);
	*/			
						if (zIndex == intPalette)
						{
							if (rIndex == dwRace && gIndex == dwSex)
							{
								pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_wNewPalette = WORD(palette | 0x04000000);
								pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucOffset = BYTE(0x18);
								pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucLength = BYTE(0x08);
								pcAvatar->m_wBasicPaletteVector++;
							}
						}	
					}
					free(palBuf);
				}
			}

			/* Determine hair style information */
			//Each hair style has a single texture associated with it and zeroed model information
			//Each hair style also has a UI Graphic associated with it
			//If a hair style is used, the style is determined by the dwHairStyle input value
			memcpy(&hairStyleCount,buf + readIndex,4);		readIndex += 4;	//Hair style count
			
			//Loop through the hair styles
			for (int hairIndex = 0; hairIndex < hairStyleCount; ++hairIndex)
			{
				DWORD	hairStyleUIGraphic;

				memcpy(&hairStyleUIGraphic,buf+readIndex,4);readIndex += 4;
				readIndex += 4;	//Unknown DWORD -- 0x00000000

				readIndex += 1;		//BYTE 11
				memcpy(&hPaletteCount,buf + readIndex,1);	readIndex += 1;	//Palette Count
				memcpy(&hTextureCount,buf + readIndex,1);	readIndex += 1;	//Texture Count
				memcpy(&hModelCount,buf + readIndex,1);		readIndex += 1;	//Model Count
				
				//Loop through the hair style palette/texture/model information
				for (int pIndex = 0; pIndex < hPaletteCount; ++pIndex)	//Palettes
				{
					WORD	newPalette;
					BYTE	palOffset;
					BYTE	palLength;

					memcpy(&newPalette,buf+readIndex,2);	readIndex += 2;
					memcpy(&palOffset,buf+readIndex,1);		readIndex += 1;
					memcpy(&palLength,buf+readIndex,1);		readIndex += 1;

					if (hairIndex == dwHairStyle)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_wNewPalette = WORD(newPalette);
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucOffset = BYTE(palOffset);
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucLength = BYTE(palLength);
							pcAvatar->m_wBasicPaletteVector++;
						}
					}
				}
				for (int tIndex = 0; tIndex < hTextureCount; ++tIndex)	//Textures
				{
					BYTE	modelIndex;
					WORD	textureMask;
					WORD	texture;

					memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
					memcpy(&textureMask,buf+readIndex,2);	readIndex += 2;
					memcpy(&texture,buf+readIndex,2);		readIndex += 2;

					if (hairIndex == dwHairStyle)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_bModelIndex = BYTE(modelIndex);
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wOldTexture = WORD(textureMask);
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wNewTexture = WORD(texture);
							pcAvatar->m_wBasicTextureVector++;
						}
					}
				}
				for (int mIndex = 0; mIndex < hModelCount; ++mIndex)		//Models
				{
					BYTE	modelIndex;
					WORD	model;

					memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
					memcpy(&model,buf+readIndex,2);			readIndex += 2;

					if (hairIndex == dwHairStyle)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							for (int index = 0; index < pcAvatar->m_wBasicModelVector; ++index)
							{
								if (pcAvatar->m_BasicVectorMod[modelIndex].m_bModelIndex == modelIndex)
								{
									pcAvatar->m_BasicVectorMod[modelIndex].m_wNewModel = WORD(model);
								}
							}
						}
					}
				}
				if (hTextureCount > 0 && hModelCount == 0)
					readIndex += 3;		//Every texture has corresponding model information (even if unused)
			}

			/* Determine eye color palette */
			//Each eye color has a single palette associated with it
			//If an eye color is used, the style is determined by the dwEyeColor input value
			memcpy(&eyeColorPaletteCount,buf + readIndex,4);	readIndex += 4;
			
			for (int eyeIndex = 0; eyeIndex < eyeColorPaletteCount; ++eyeIndex)
			{
				WORD	newPalette;
				BYTE	palOffset;
				BYTE	palLength;
				
				memcpy(&newPalette,buf+readIndex,2);		readIndex += 2;
				memcpy(&palOffset,buf+readIndex,1);			readIndex += 1;
				memcpy(&palLength,buf+readIndex,1);			readIndex += 1;

				if (eyeIndex == dwEyeColor)
				{
					if (rIndex == dwRace && gIndex == dwSex)
					{
						pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_wNewPalette = WORD(newPalette);
						pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucOffset = BYTE(palOffset);
						pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucLength = BYTE(palLength);
						pcAvatar->m_wBasicPaletteVector++;
					}
				}
			}

			/* Determine forehead information */
			//Each forehead style has two textures associated with it and zeroed model information
			//Only one texture is used: the first is for non-bald characters; the second is for bald characters
			//If a forehead style is used, the style is determined by the dwForeheadTexture input value
			memcpy(&foreheadTextureCount,buf + readIndex,4);	readIndex += 4;	//Forehead style count
			
			//Loop through the forehead styles
			for (int foreheadIndex = 0; foreheadIndex < foreheadTextureCount; ++foreheadIndex)
			{
				DWORD	unknownUIGraphic;

				memcpy(&unknownUIGraphic,buf+readIndex,4);		readIndex += 4;
				readIndex += 4;	//Unknown DWORD -- 0x00000000

				//Forehead styles consist of two sets of palette/texture/model information
				for (int i = 0; i < 2; ++i)
				{
					readIndex += 1;		//BYTE 11
					memcpy(&hPaletteCount,buf + readIndex,1);	readIndex += 1;	//Palette Count
					memcpy(&hTextureCount,buf + readIndex,1);	readIndex += 1;	//Texture Count
					memcpy(&hModelCount,buf + readIndex,1);		readIndex += 1;	//Model Count
					
					//Loop through the forehead style palette/texture/model information
					for (int pIndex = 0; pIndex < hPaletteCount; ++pIndex)	//Palettes
					{
						WORD	newPalette;
						BYTE	palOffset;
						BYTE	palLength;

						memcpy(&newPalette,buf+readIndex,2);	readIndex += 2;
						memcpy(&palOffset,buf+readIndex,1);		readIndex += 1;
						memcpy(&palLength,buf+readIndex,1);		readIndex += 1;

						if (foreheadIndex == dwForeheadTexture)
						{
							if (rIndex == dwRace && gIndex == dwSex)
							{
								if (i == 0 && dwHairStyle != 3 || i == 1 && dwHairStyle == 3)
								{
									pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_wNewPalette = WORD(newPalette);
									pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucOffset = BYTE(palOffset);
									pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucLength = BYTE(palLength);
									pcAvatar->m_wBasicPaletteVector++;
								}
							}
						}
					}
					for (int tIndex = 0; tIndex < hTextureCount; ++tIndex)	//Textures
					{
						BYTE	modelIndex;
						WORD	textureMask;
						WORD	texture;

						memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
						memcpy(&textureMask,buf+readIndex,2);	readIndex += 2;
						memcpy(&texture,buf+readIndex,2);		readIndex += 2;

						if (foreheadIndex == dwForeheadTexture)
						{
							if (rIndex == dwRace && gIndex == dwSex)
							{
								if (i == 0 && dwHairStyle != 3 || i == 1 && dwHairStyle == 3)
								{
									pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_bModelIndex = BYTE(modelIndex);
									pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wOldTexture = WORD(textureMask);
									pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wNewTexture = WORD(texture);
									pcAvatar->m_wBasicTextureVector++;
								}
							}
						}
					}
					for (int mIndex = 0; mIndex < hModelCount; ++mIndex)	//Models
					{
						BYTE	modelIndex;
						WORD	model;

						memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
						memcpy(&model,buf+readIndex,2);			readIndex += 2;

						if (foreheadIndex == dwForeheadTexture)
						{
							if (rIndex == dwRace && gIndex == dwSex)
							{
								if (i == 0 && dwHairStyle != 3 || i == 1 && dwHairStyle == 3)
								{
									pcAvatar->m_BasicVectorMod[pcAvatar->m_wBasicModelVector].m_bModelIndex = modelIndex;
									pcAvatar->m_BasicVectorMod[pcAvatar->m_wBasicModelVector].m_wNewModel = model;
									pcAvatar->m_wBasicModelVector++;
								}
							}
						}
					}
					if (hTextureCount > 0 && hModelCount == 0)
						readIndex += 3;		//Every texture has corresponding model information (even if unused)
				}
			}

			/* Determine nose information */
			//Each nose style has a single texture associated with it and zeroed model information
			//If a nose style is used, the style is determined by the dwNoseTexture input value
			memcpy(&noseTextureCount,buf + readIndex,4);	readIndex += 4;	//Nose style count
			
			//Loop through the nose styles
			for (int noseIndex = 0; noseIndex < noseTextureCount; ++noseIndex)
			{
				readIndex += 4;	//Unknown DWORD -- 0x00000000

				readIndex += 1;		//BYTE 11
				memcpy(&hPaletteCount,buf + readIndex,1);	readIndex += 1;	//Palette Count
				memcpy(&hTextureCount,buf + readIndex,1);	readIndex += 1;	//Texture Count
				memcpy(&hModelCount,buf + readIndex,1);		readIndex += 1;	//Model Count
				
				//Loop through the nose style palette/texture/model information
				for (int pIndex = 0; pIndex < hPaletteCount; ++pIndex)	//Palettes
				{
					WORD	newPalette;
					BYTE	palOffset;
					BYTE	palLength;

					memcpy(&newPalette,buf+readIndex,2);	readIndex += 2;
					memcpy(&palOffset,buf+readIndex,1);		readIndex += 1;
					memcpy(&palLength,buf+readIndex,1);		readIndex += 1;

					if (noseIndex == dwNoseTexture)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_wNewPalette = WORD(newPalette);
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucOffset = BYTE(palOffset);
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucLength = BYTE(palLength);
							pcAvatar->m_wBasicPaletteVector++;
						}
					}
				}
				for (int tIndex = 0; tIndex < hTextureCount; ++tIndex)	//Textures
				{
					BYTE	modelIndex;
					WORD	textureMask;
					WORD	texture;

					memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
					memcpy(&textureMask,buf+readIndex,2);	readIndex += 2;
					memcpy(&texture,buf+readIndex,2);		readIndex += 2;

					if (noseIndex == dwNoseTexture)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_bModelIndex = BYTE(modelIndex);
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wOldTexture = WORD(textureMask);
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wNewTexture = WORD(texture);
							pcAvatar->m_wBasicTextureVector++;
						}
					}
				}
				for (int mIndex = 0; mIndex < hModelCount; ++mIndex)		//Models
				{
					BYTE	modelIndex;
					WORD	model;

					memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
					memcpy(&model,buf+readIndex,2);			readIndex += 2;

					if (noseIndex == dwNoseTexture)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							pcAvatar->m_BasicVectorMod[pcAvatar->m_wBasicModelVector].m_bModelIndex = modelIndex;
							pcAvatar->m_BasicVectorMod[pcAvatar->m_wBasicModelVector].m_wNewModel = model;
							pcAvatar->m_wBasicModelVector++;
						}
					}
				}
				if (hTextureCount > 0 && hModelCount == 0)
					readIndex += 3;		//Every texture has corresponding model information (even if unused)
			}

			/* Determine chin information */
			//Each chin style has a single texture associated with it and zeroed model information
			//If a chin style is used, the style is determined by the dwChinTexture input value
			memcpy(&chinTextureCount,buf + readIndex,4);	readIndex += 4;	//Chin style count
			
			//Loop through the chin styles
			for (int chinIndex = 0; chinIndex < chinTextureCount; ++chinIndex)
			{
				readIndex += 4;	//Unknown DWORD -- 0x00000000

				readIndex += 1;		//BYTE 11
				memcpy(&hPaletteCount,buf + readIndex,1);	readIndex += 1;	//Palette Count
				memcpy(&hTextureCount,buf + readIndex,1);	readIndex += 1;	//Texture Count
				memcpy(&hModelCount,buf + readIndex,1);		readIndex += 1;	//Model Count
				
				//Loop through the chin style palette/texture/model information
				for (int pIndex = 0; pIndex < hPaletteCount; ++pIndex)	//Palettes
				{
					WORD	newPalette;
					BYTE	palOffset;
					BYTE	palLength;

					memcpy(&newPalette,buf+readIndex,2);	readIndex += 2;
					memcpy(&palOffset,buf+readIndex,1);		readIndex += 1;
					memcpy(&palLength,buf+readIndex,1);		readIndex += 1;

					if (chinIndex == dwChinTexture)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_wNewPalette = WORD(newPalette);
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucOffset = BYTE(palOffset);
							pcAvatar->m_BasicVectorPal[pcAvatar->m_wBasicPaletteVector].m_ucLength = BYTE(palLength);
							pcAvatar->m_wBasicPaletteVector++;
						}
					}
				}
				for (int tIndex = 0; tIndex < hTextureCount; ++tIndex)	//Textures
				{
					BYTE	modelIndex;
					WORD	textureMask;
					WORD	texture;

					memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
					memcpy(&textureMask,buf+readIndex,2);	readIndex += 2;
					memcpy(&texture,buf+readIndex,2);		readIndex += 2;

					if (chinIndex == dwChinTexture)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_bModelIndex = BYTE(modelIndex);
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wOldTexture = WORD(textureMask);
							pcAvatar->m_BasicVectorTex[pcAvatar->m_wBasicTextureVector].m_wNewTexture = WORD(texture);
							pcAvatar->m_wBasicTextureVector++;
						}
					}
				}
				for (int mIndex = 0; mIndex < hModelCount; ++mIndex)	//Models
				{
					BYTE	modelIndex;
					WORD	model;

					memcpy(&modelIndex,buf+readIndex,1);	readIndex += 1;
					memcpy(&model,buf+readIndex,2);			readIndex += 2;

					if (chinIndex == dwChinTexture)
					{
						if (rIndex == dwRace && gIndex == dwSex)
						{
							pcAvatar->m_BasicVectorMod[pcAvatar->m_wBasicModelVector].m_bModelIndex = modelIndex;
							pcAvatar->m_BasicVectorMod[pcAvatar->m_wBasicModelVector].m_wNewModel = model;
							pcAvatar->m_wBasicModelVector++;
						}
					}
				}
				if (hTextureCount > 0 && hModelCount == 0)
					readIndex += 3;		//Every texture has corresponding model information (even if unused)
			}

			/* Apparel Items */
			//There are always four types of starting clothing (headgear, shirts, trousers, footwear)
			//Each type has a count of its respective number of items.
			//Each item has a name, reference to its Complex Model, and an unknown DWORD

			//Loop through the headgear
			DWORD	headgearCount;
			DWORD	shirtCount;
			DWORD	trousersCount;
			DWORD	footwearCount;

			memcpy(&headgearCount,buf + readIndex,4);			readIndex += 4;
			
			for	(int headgearIndex = 0; headgearIndex < headgearCount; ++headgearIndex)
			{
				WORD	apparelNameLength;
				char	apparelName[40];
				DWORD	complexModelID;
				DWORD	unknown1;
				
				memcpy(&apparelNameLength,buf + readIndex,2);			readIndex += 2;
				memcpy(&apparelName,buf + readIndex,apparelNameLength);	readIndex += apparelNameLength;

				while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding

				memcpy(&complexModelID,buf + readIndex,4);		readIndex += 4;	//Complex Model (0x10000000 value)
				memcpy(&unknown1,buf + readIndex,4);			readIndex += 4;
			}

			memcpy(&shirtCount,buf + readIndex,4);				readIndex += 4;

			//Loop through the shirts
			for	(int shirtIndex = 0; shirtIndex < shirtCount; ++shirtIndex)
			{
				WORD	apparelNameLength;
				char	apparelName[40];
				DWORD	complexModelID;
				DWORD	unknown1;
				
				memcpy(&apparelNameLength,buf + readIndex,2);				readIndex += 2;
				memcpy(&apparelName,buf + readIndex,apparelNameLength);		readIndex += apparelNameLength;

				while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding

				memcpy(&complexModelID,buf + readIndex,4);					readIndex += 4;	//Complex Model (0x10000000 value)
				memcpy(&unknown1,buf + readIndex,4);						readIndex += 4;
			}

			memcpy(&trousersCount,buf + readIndex,4);			readIndex += 4;

			//Loop through the trousers
			for	(int trousersIndex = 0; trousersIndex < trousersCount; ++trousersIndex)
			{
				WORD	apparelNameLength;
				char	apparelName[40];
				DWORD	complexModelID;
				DWORD	unknown1;
				
				memcpy(&apparelNameLength,buf + readIndex,2);			readIndex += 2;
				memcpy(&apparelName,buf + readIndex,apparelNameLength);	readIndex += apparelNameLength;

				while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding

				memcpy(&complexModelID,buf + readIndex,4);					readIndex += 4;	//Complex Model (0x10000000 value)
				memcpy(&unknown1,buf + readIndex,4);						readIndex += 4;
			}

			memcpy(&footwearCount,buf + readIndex,4);		readIndex += 4;

			//Loop through the footwear
			for	(int footwearIndex = 0; footwearIndex < footwearCount; ++footwearIndex)
			{
				WORD	apparelNameLength;
				char	apparelName[40];
				DWORD	complexModelID;
				DWORD	unknown1;
				
				memcpy(&apparelNameLength,buf + readIndex,2);			readIndex += 2;
				memcpy(&apparelName,buf + readIndex,apparelNameLength);	readIndex += apparelNameLength;

				while (readIndex % 4 != 0) { readIndex++; }	//DWORD padding

				memcpy(&complexModelID,buf + readIndex,4);					readIndex += 4;	//Complex Model (0x10000000 value)
				memcpy(&unknown1,buf + readIndex,4);						readIndex += 4;
			}

			DWORD	apparelColorsCount;
//			DWORD	apparelColorConstant;

			memcpy(&apparelColorsCount,buf + readIndex,4);		readIndex += 4;
			readIndex +=4;	//Unknown DWORD (0x0000000E)

			for	(int colorIndex = 0; colorIndex < apparelColorsCount; ++colorIndex)
			{
				DWORD	colorNum;
				memcpy(&colorNum,buf + readIndex,4);			readIndex += 4;
			}
		}
	}
	free(buf);
}

/**
 *	Handles the loading of item models (palette, texture, and model information).
 *
 *	Called whenever an object's p/t/m information should be initialized or updated.
 *
 *	@param *pcObject - A pointer to the object whose p/t/m information will be initialized or updated.
 *	@param dwModelID - The item model to be used (world, male, female) for the object.
 *	@param dwColorID - The color of the object.
 *	@param dblColorValue - The color shade of the object.
 */
void cPortalDat::LoadItemModel( cObject *pcObject, DWORD dwModelID, DWORD dwColorID, double dblColorValue )
{
	cItemModels *pcModel = cItemModels::FindModel(( pcObject )->GetItemModelID());
	//pcModel		- Item model instantiation; used to fetch item variables
	//dwModelID		- 0x00000001 = male; 0x0000004E = female; other
	//dwColorID		- color ID number; determines the CLUT list looked-up
	//dwColorValue	- the particlar CLUT from the clut list
	if (!pcModel->m_clothingModelLoaded || dwColorID != NULL)
	{
		if (dwModelID == 0x02000001 || dwModelID == 0x0200004E)
		{
			pcModel->m_bWearPaletteChange = 0;
			pcModel->m_bWearTextureChange = 0;
			pcModel->m_bWearModelChange = 0;
		}

		char	portalFile[9];
		sprintf(portalFile,"%08x",pcModel->m_PortalLinker);

		/* Complex Object (0x10000000) Algorithm */
		//This algorithm fetches item model data from the portal.dat,
		//The data can be used to populate the item-based palette, texture, and model informtion.

	//	char	szData[16];
		UCHAR	*buf;

		//DWORD	fileNum;
		WORD	modelCount;
		WORD	modelListUnknown;

		cPortalDat::Load_PortalDat(buf, portalFile);
		int readIndex = 0;

		//memcpy(&fileNum,buf,4);
		readIndex += 4;	//the first 4 bytes list the index for which we searched

		memcpy(&modelCount,buf + readIndex,2);
		readIndex += 2;
		memcpy(&modelListUnknown,buf + readIndex,2);
		readIndex += 2;
	/*
		sprintf(szData,"%04x %d\r\n",modelCount,d);
		UpdateConsole(szData);
		sprintf(szData,"%04x\r\n",modelListUnknown);
		UpdateConsole(szData);
	*/
		//Loop through the models types (as applicable:  world model, male wearable model, female wearable model)
		for (int xIndex = 0; xIndex < modelCount; ++xIndex)
		{
			DWORD modelComplexID;
			DWORD modelAmount;	

	//		UpdateConsole("\r\n");
			memcpy(&modelComplexID,buf+readIndex,4);
			readIndex += 4;
			memcpy(&modelAmount,buf+readIndex,4);
			readIndex += 4;
	/*
			sprintf(szData,"Complex Model ID: %08x\r\n",modelComplexID);
			UpdateConsole(szData);
			sprintf(szData,"Model Amount: %08x\r\n",modelAmount);
			UpdateConsole(szData);
	*/
			int mIndex = 0;
			int tIndex = 0;
			//Loop through the model vectors (double of index ID(?) and sub model (0x01000000 values))
			for (int yIndex = 0; yIndex < modelAmount; ++yIndex)
			{
				DWORD modelIndex;		//body part
				/*
				0x01 - Left Upper Leg
				0x02 - Left Lower leg
				0x03 - Left Shin
				0x04 - Left Foot
				0x05 - Right Upper leg
				0x06 - Right Lower Leg
				0x07 - Right Shin
				0x08 - Right Foot
				0x09 - Chest
				0x0A - Upper Arm (shoulder) - Left
				0x0B - Wrist - Left Arm
				0x0C - Left Hand
				0x0D - Upper Arm (shoulder) - Right
				0x0E - Wrist - Right Arm
				0x0F - Right Hand
				0x10 - Head
				*/
				DWORD modelID;			//location of model in portal.dat
				DWORD modelTextureCount;

				memcpy(&modelIndex,buf+readIndex,4);
				readIndex += 4;
				memcpy(&modelID,buf+readIndex,4);
				readIndex += 4;
				memcpy(&modelTextureCount,buf+readIndex,4);
				readIndex += 4;
	/*
				sprintf(szData,"Model Index: %08x\r\n",modelIndex);
				UpdateConsole(szData);
				sprintf(szData,"Model ID: %08x\r\n",modelID);
				UpdateConsole(szData);
				sprintf(szData,"Model Texture Count: %08x\r\n",modelTextureCount);
				UpdateConsole(szData);
	*/
				//Update item model information
				if (modelComplexID == dwModelID)
				{
					if (dwModelID == 0x02000001 || dwModelID == 0x0200004E)
					{
						//fetch item model data for characters
						pcModel->m_WearVectorMod[mIndex].m_bModelIndex = BYTE(modelIndex);
						pcModel->m_WearVectorMod[mIndex].m_wNewModel = WORD(modelID | 0x01000000);
						pcModel->m_bWearModelChange = pcModel->m_bWearModelChange + 1;
					}
					else
					{
						//fetch item model data
						pcModel->m_vectorMod[mIndex].m_bModelIndex = BYTE(modelIndex);
						pcModel->m_vectorMod[mIndex].m_wNewModel = WORD(modelID | 0x01000000);
						pcModel->m_bModelChange = pcModel->m_bModelChange + 1;
					}
					mIndex++;
				}

				//Loop through the textures (0x04000000 values) to be applied to the given submodel (double of texture mask and texture)
				for (int zIndex = 0; zIndex < modelTextureCount; ++zIndex)
				{
					DWORD modelTextureMask;
					DWORD modelTexture;

					memcpy(&modelTextureMask,buf+readIndex,4);
					readIndex += 4;
					memcpy(&modelTexture,buf+readIndex,4);
					readIndex += 4;
	/*
					sprintf(szData,"Model Texture Mask: %08x\r\n",modelTextureMask);
					UpdateConsole(szData);
					sprintf(szData,"Model Texture: %08x\r\n",modelTexture);
					UpdateConsole(szData);
	*/			
					//Update item texture information
					if (modelComplexID == dwModelID)
					{
						if (dwModelID == 0x02000001 || dwModelID == 0x0200004E)
						{
							//fetch item texture data for characters
							pcModel->m_WearVectorTex[tIndex].m_bModelIndex = BYTE(modelIndex);
							pcModel->m_WearVectorTex[tIndex].m_wOldTexture = WORD(modelTextureMask | 0x05000000);
							pcModel->m_WearVectorTex[tIndex].m_wNewTexture = WORD(modelTexture | 0x05000000);
							pcModel->m_bWearTextureChange = pcModel->m_bWearTextureChange + 1;
						}
						else
						{
							pcModel->m_vectorTex[tIndex].m_bModelIndex = BYTE(modelIndex);
							pcModel->m_vectorTex[tIndex].m_wOldTexture = WORD(modelTextureMask | 0x05000000);
							pcModel->m_vectorTex[tIndex].m_wNewTexture = WORD(modelTexture | 0x05000000);
							pcModel->m_bTextureChange = pcModel->m_bTextureChange + 1;
						}
						tIndex++;
					}
				}
			}
		}

	//	UpdateConsole("\r\n");

		WORD altModelCount;
		WORD unknown;

		memcpy(&altModelCount,buf+readIndex,2);
		readIndex += 2;
		memcpy(&unknown,buf+readIndex,2);
		readIndex += 2;
	/*
		sprintf(szData,"Alt Model Count: %08x\r\n",altModelCount);
		UpdateConsole(szData);
		sprintf(szData,"Unknown: %08x\r\n",unknown);
		UpdateConsole(szData);
	*/
		for (int wIndex = 0; wIndex < altModelCount; ++wIndex)
		{
	//		UpdateConsole("\r\n");
			int pIndex = 0;

			DWORD altModelComplexID;	//Color ID
			// General colors //
			//Similar colors are different hues
			//Colors 1-18 are available during character selection
			/*
			 1 - blue
			 2 - blue
			 3 - violet
			 4 - brown
			 5 - blue
			 6 - brown
			 7 - green
			 8 - green
			 9 - white/gray/black
			10 - blue
			11 - brown
			12 - blue
			13 - violet
			14 - red
			15 - red
			16 - red
			17 - yellow
			18 - gold
			84 - green
			85 - red
			86 - yellow
			87 - red
			88 - blue
			89 - green
			90 - black
			91 - blue
			92 - violet
			93 - white/gray/black
			*/
			DWORD altModelIcon;
			DWORD paletteAmount;

			memcpy(&altModelComplexID,buf+readIndex,4);
			readIndex += 4;
			memcpy(&altModelIcon,buf+readIndex,4);		//0x06000000 value
			readIndex += 4;
			memcpy(&paletteAmount,buf+readIndex,4);
			readIndex += 4;

			if (dwColorID)
				if (dwColorID == altModelComplexID)
					pcObject->m_wIcon = altModelIcon;

	/*		sprintf(szData,"Alt Complex Model ID: %08x\r\n",altModelComplexID);
			UpdateConsole(szData);
			sprintf(szData,"Alt Model Icon: %08x\r\n",altModelIcon);
			UpdateConsole(szData);
			sprintf(szData,"Palette Amount: %08x\r\n",paletteAmount);
			UpdateConsole(szData);
	*/
			//Loop through the model palette list
			for (int xIndex = 0; xIndex < paletteAmount; ++xIndex)
			{
				DWORD paletteVectorAmount;

				memcpy(&paletteVectorAmount,buf+readIndex,4);
				readIndex += 4;
	/*		
				sprintf(szData,"Palette Vector Amount: %08x\r\n",paletteVectorAmount);
				UpdateConsole(szData);
	*/			
				//Loop through the specific palette vector (trio of palette offset, palette length, and CLUT list)
				for (int yIndex = 0; yIndex < paletteVectorAmount; ++yIndex)
				{
					DWORD paletteVectorOffset;
					DWORD paletteVectorValue;

					memcpy(&paletteVectorOffset,buf+readIndex,4);
					readIndex += 4;
					memcpy(&paletteVectorValue,buf+readIndex,4);
					readIndex += 4;
	/*
					sprintf(szData,"Palette Vector Offset: %08x\r\n",paletteVectorOffset);
					UpdateConsole(szData);
					sprintf(szData,"Palette Vector Value: %08x\r\n",paletteVectorValue);
					UpdateConsole(szData);
	*/			
					if (dwColorID)
					{
						if (dwColorID == altModelComplexID)
						{
							if (dwModelID == 0x02000001 || dwModelID == 0x0200004E)
							{
								//fetch item texture data for characters
								pcObject->m_WearVectorPal[pIndex].m_ucOffset = BYTE(paletteVectorOffset);
								pcObject->m_WearVectorPal[pIndex].m_ucLength = BYTE(paletteVectorValue);
								pcObject->m_bWearPaletteChange = pcObject->m_bWearPaletteChange + 1;
							}
							else
							{
								pcObject->m_vectorPal[pIndex].m_ucOffset = BYTE(paletteVectorOffset);
								pcObject->m_vectorPal[pIndex].m_ucLength = BYTE(paletteVectorValue);
								pcObject->m_bPaletteChange = pcObject->m_bPaletteChange + 1;
							}
							pIndex++;
						}
					}
				}

				DWORD paletteID;

				memcpy(&paletteID,buf+readIndex,4);
				readIndex += 4;
	/*
				sprintf(szData,"Palette ID: %08x\r\n",paletteID);
				UpdateConsole(szData);
	*/		
				char	szPalFile[9];
				sprintf(szPalFile,"%08x",paletteID);
				UCHAR	*palBuf;

				//DWORD	paletteFileNum;
				DWORD	paletteCount;

				//Load palette data from the portal.dat file
				cPortalDat::Load_PortalDat(palBuf, szPalFile);

				int palReadIndex = 0;
				palReadIndex += 4;	//the first 4 bytes list the index for which we searched

				memcpy(&paletteCount,palBuf+palReadIndex,4);
				palReadIndex += 4;
	/*
				sprintf(szData,"Palette Count: %08x\r\n",paletteCount);
				UpdateConsole(szData);
	*/
				//Calculate the palette to be used
				int intPalette = CalcPalette(paletteCount,dblColorValue);

				//Loop through the specific palettes (0x04000000 values)
				for (int zIndex = 0; zIndex < paletteCount; ++zIndex)
				{
					DWORD palette;
					memcpy(&palette,palBuf+palReadIndex,4);
					palReadIndex += 4;
	/*
					sprintf(szData,"Palette: %08x\r\n",palette);
					UpdateConsole(szData);
	*/			
					if (dwColorID == altModelComplexID)
					{
						if (zIndex == intPalette)
						{
							if (dwModelID == 0x02000001 || dwModelID == 0x0200004E)
							{
								pcObject->m_WearVectorPal[xIndex].m_wNewPalette = WORD(palette | 0x04000000);
							}
							else
							{
								pcObject->m_vectorPal[xIndex].m_wNewPalette = WORD(palette | 0x04000000);
							}
						}
					}
				}
				free(palBuf);
			}
		}
		free(buf);
		
		if (dwModelID == 0x02000001 || dwModelID == 0x0200004E)
			pcModel->m_clothingModelLoaded = true;
	}
}


/**
 *	Determines the CLUT (0x04000000 object in the portal.dat) to be used.
 *
 *	The palette to use is determined by rounding the value of numPalettes minus one (as the
 *	palettes are zero-indexed) multiplied by the palValue value.
 *
 *	@param numPalettes - The number of palettes in the palette list.
 *	@param palValue - The shading value used to determine the palette (between 0 and 1).
 *
 *	@return palette - The palette index (beginning with 0) to use.
 */
int cPortalDat::CalcPalette( int numPalettes, double palValue )
{
	if (palValue == -1)
	{
		return 0;
	}
	else
	{
		int palette = floor( (double)(palValue * (numPalettes - 1)) + .5 );
		if (palette < 0) palette = 0;
		if (palette > (numPalettes - 1)) palette = (numPalettes - 1);
		return palette;
	}
}