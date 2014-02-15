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
 *	@file MagicSystem.h
 */

#if !defined(AFX_MAGICSYSTEM_H__B6F883BA_AC90_4FC1_818D_9CB14921DEC4__INCLUDED_)
#define AFX_MAGICSYSTEM_H__B6F883BA_AC90_4FC1_818D_9CB14921DEC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Shared.h"
#include "Message.h"

class MagicSystem  
{
public:
	int m_intSpellSpeed;
	int m_intSpell_lvl;
	DWORD m_dwSpellID;
	DWORD m_dwCastGUID;
	DWORD m_dwTargetGUID;
	MagicSystem();
	virtual ~MagicSystem();

};

#endif // !defined(AFX_MAGICSYSTEM_H__B6F883BA_AC90_4FC1_818D_9CB14921DEC4__INCLUDED_)