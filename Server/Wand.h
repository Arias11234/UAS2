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

#ifndef __WAND_H
#define __WAND_H

#include "Object.h"
#define DAMAGE_MAGIC	0x2
//Cubem0j0: don't need this struct...
/*
struct cWandInfo
{
	DWORD		m_dwDamageType;
	DWORD		m_dwSkillUsed;
	int			m_iLowerDamage;
	int			m_iUpperDamage;
	int			m_iAttackModifier;
	int			m_iDefenseModifier;
	int			m_iSpeed;

	cWandInfo( ) 
		:	m_dwSkillUsed		( SKILL_WAR_MAGIC ), 
			m_dwDamageType		( DAMAGE_MAGIC ), 
			m_iSpeed			( 20 ), 
			m_iLowerDamage		( 1 ), 
			m_iUpperDamage		( 4 ),
			m_iAttackModifier	( 0 ), 
			m_iDefenseModifier	( 0 )
	{
	}
};
*//*
class cWand : public cAbiotic
{
public:
	cWand( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwWeight, DWORD dwValue, BOOL fIsOwned = FALSE, DWORD dwContainer = 0 )
	{
		m_dwGUID			= dwGUID;
		m_bInventorySequence= -1;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_dwModel			= dwModel;
		m_wIcon				= dwIcon;
		m_fIsOwned			= FALSE;
		m_dwContainer		= dwContainer;
		m_dwWeight			= dwWeight;
		m_dwValue			= dwValue;
		m_fEquippable		= TRUE;
		m_fIsStackable		= FALSE;
		m_fSelectable		= TRUE;
		m_flScale			= flScale;
		m_fEquipped			= 0;
		m_wNumLogins		= 0x3C;
		m_wPositionSequence	= 0;
		m_wNumPortals		= 0;

		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	cWand( ) {}

	// Use the Abiotic createpacket( ) for now
	cMessage	CreatePacket( );
	void Assess( cClient *pcAsseser );

private:

};
*/
#endif	// #ifndef __WEAPON_H