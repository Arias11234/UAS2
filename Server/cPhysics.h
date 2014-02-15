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
 *	@file cPhysics.h
 */

#if !defined(AFX_CPHYSICS_H__87BF30D9_6373_47F7_AAC7_56024CA96079__INCLUDED_)
#define AFX_CPHYSICS_H__87BF30D9_6373_47F7_AAC7_56024CA96079__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "uas.h"

///////////////////////////////////////
//  Math Functions - Quaternion
///////////////////////////////////////
struct quaternion
{
	float x, y, z, w;
};

struct vector
{
	float x, y, z;
};

///////////////////////////////////////

class cPhysics  
{
public:
	cPhysics();
	virtual ~cPhysics();

	static float		length				( quaternion quat );
	static quaternion	normalize			( quaternion quat );
	static quaternion	conjugate			( quaternion quat );
	static quaternion	mult				( quaternion A, quaternion B );
	static void			Rotate				( float Angle, float x, float y, float z, vector& View );
	static float		GetRange			( cLocation usrLoc, cLocation tarLoc );
	static float		Get3DRange			( cLocation usrLoc, cLocation tarLoc );
	static cVelocity	GetTargetVelocity	( cLocation usrLoc, cLocation tarLoc );
	static cLocation	VelocityMove		( cLocation usrLoc, cVelocity tarVel, float flSpeed );
	static float		GetLineDistance		( cLocation OldLoc, cLocation NewLoc, cLocation TarLoc );
	static float		Get3DLineDistance	( cLocation OldLoc, cLocation NewLoc, cLocation TarLoc );
	static float		GetHeadingTarget	( cLocation usrLoc, cLocation tarLoc );
	static float		GetAvatarHeading	( cLocation usrLoc );
	static float		GetHeadingDifference( float flUserHeading, float flTargetHeading );
	static float		GetLandZ			( cLocation usrLoc );
};

#endif // !defined(AFX_CPHYSICS_H__87BF30D9_6373_47F7_AAC7_56024CA96079__INCLUDED_)