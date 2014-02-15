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
 *	@file cPhysics.cpp
 *	Implements functionality for item models.
 *
 *	Item models comprise the generic traits shared among a type of object with a particular model.
 *
 *	Special thanks to Greg Kusnick for his work on the landblock triangulation algorithm.
 */

#include "cPhysics.h"
#include "WorldManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/***************
 *	constructors/destructors
 **************/

cPhysics::cPhysics()
{

}

cPhysics::~cPhysics()
{

}

float cPhysics::length(quaternion quat)
{
	return sqrt(quat.x*quat.x + quat.y*quat.y + quat.z*quat.z + quat.w*quat.w);
}

quaternion cPhysics::normalize(quaternion quat)
{
  float L = length(quat);

  quat.x /= L;
  quat.y /= L;
  quat.z /= L;
  quat.w /= L;

  return quat;
}

quaternion cPhysics::conjugate(quaternion quat)
{
  quat.x = -quat.x;
  quat.y = -quat.y;
  quat.z = -quat.z;
  return quat;
}

quaternion cPhysics::mult(quaternion A, quaternion B)
{
  quaternion C;

  C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;
  C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;
  C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;
  C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;
  return C;
}

void cPhysics::Rotate(float Angle, float x, float y, float z, vector& View)
{
  quaternion temp, quat_view, result;

  temp.x = x * sin(Angle/2);
  temp.y = y * sin(Angle/2);
  temp.z = z * sin(Angle/2);
  temp.w = cos(Angle/2);

  quat_view.x = View.x;
  quat_view.y = View.y;
  quat_view.z = View.z;
  quat_view.w = 0;

  result = mult(mult(temp, quat_view), conjugate(temp));

  View.x = result.x;
  View.y = result.y;
  View.z = result.z;

}

float cPhysics::GetLineDistance ( cLocation OldLoc, cLocation NewLoc, cLocation TarLoc ) //agentsparrow
{
	float nsCoord1,ewCoord1,nsCoord2,ewCoord2;
	float nsTarCoord,ewTarCoord,flDistance;

	  nsCoord1 = (((((OldLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(OldLoc.m_flY / 24) - 1027.5; 
      ewCoord1 = (((((OldLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(OldLoc.m_flX / 24) - 1027.5;
	  nsCoord2 = (((((NewLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(NewLoc.m_flY / 24) - 1027.5; 
      ewCoord2 = (((((NewLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(NewLoc.m_flX / 24) - 1027.5;
	  nsTarCoord = (((((TarLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(TarLoc.m_flY / 24) - 1027.5; 
      ewTarCoord = (((((TarLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(TarLoc.m_flX / 24) - 1027.5;

	  float flNumerator = (ewTarCoord-ewCoord1)*(ewCoord2-ewCoord1) + (nsTarCoord-nsCoord1)*(nsCoord2-nsCoord1);
	  float flDenomenator = (ewCoord2-ewCoord1)*(ewCoord2-ewCoord1) + (nsCoord2-nsCoord1)*(nsCoord2-nsCoord1);
	  float flR = flNumerator / flDenomenator;

	  if ( (flR >= 0.0f) && (flR <= 1.0f) )
	  {
		  float flS = ((nsCoord1-nsTarCoord)*(ewCoord2-ewCoord1)-(ewCoord1-ewTarCoord)*(nsCoord2-nsCoord1)) / flDenomenator;
		  flDistance = fabs(flS)*sqrt(flDenomenator);
		  return flDistance;
	  }
	  else
	  {
		  float flDist1 = (ewTarCoord-ewCoord1)*(ewTarCoord-ewCoord1) + (nsTarCoord-nsCoord1)*(nsTarCoord-nsCoord1);
		  float flDist2 = (ewTarCoord-ewCoord2)*(ewTarCoord-ewCoord2) + (nsTarCoord-nsCoord2)*(nsTarCoord-nsCoord2);
		  
		  if (flDist1 < flDist2)
		  {
			  flDistance = sqrt(flDist1);
		  }
		  else
		  {
			  flDistance = sqrt(flDist2);
		  }
	  }
	  return flDistance;
}

float cPhysics::Get3DLineDistance ( cLocation OldLoc, cLocation NewLoc, cLocation TarLoc ) //agentsparrow
{
	float nsCoord1,ewCoord1,nsCoord2,ewCoord2;
	float nsTarCoord,ewTarCoord,OldZ,NewZ,TarZ,flDistance;

	  nsCoord1 = (((((OldLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(OldLoc.m_flY / 24) - 1027.5; 
      ewCoord1 = (((((OldLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(OldLoc.m_flX / 24) - 1027.5;
	  nsCoord2 = (((((NewLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(NewLoc.m_flY / 24) - 1027.5; 
      ewCoord2 = (((((NewLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(NewLoc.m_flX / 24) - 1027.5;
	  nsTarCoord = (((((TarLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(TarLoc.m_flY / 24) - 1027.5; 
      ewTarCoord = (((((TarLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(TarLoc.m_flX / 24) - 1027.5;
	  OldZ = OldLoc.m_flZ/24;
	  NewZ = NewLoc.m_flZ/24;
	  TarZ = TarLoc.m_flZ/24;

	  float flNumerator = (ewTarCoord-ewCoord1)*(ewCoord2-ewCoord1) + (nsTarCoord-nsCoord1)*(nsCoord2-nsCoord1) + (TarZ-OldZ)*(NewZ-OldZ);
	  float flDenomenator = (ewCoord2-ewCoord1)*(ewCoord2-ewCoord1) + (nsCoord2-nsCoord1)*(nsCoord2-nsCoord1) + (NewZ-OldZ)*(NewZ-OldZ);
	  float flR = flNumerator / flDenomenator;

	  if ( (flR >= 0.0f) && (flR <= 1.0f) )
	  {
		  float IntX = ewCoord1 + flR * (ewCoord2-ewCoord1);
		  float IntY = nsCoord1 + flR * (nsCoord2-nsCoord1);
		  float IntZ = OldZ + flR * (NewZ-OldZ);
		  flDistance = sqrt((ewTarCoord-IntX)*(ewTarCoord-IntX) + (nsTarCoord-IntY)*(nsTarCoord-IntY) + (TarZ-IntZ)*(TarZ-IntZ));
		  return flDistance;
	  }
	  else
	  {
		  float flDist1 = (ewTarCoord-ewCoord1)*(ewTarCoord-ewCoord1) + (nsTarCoord-nsCoord1)*(nsTarCoord-nsCoord1) + (TarZ-OldZ)*(TarZ-OldZ);
		  float flDist2 = (ewTarCoord-ewCoord2)*(ewTarCoord-ewCoord2) + (nsTarCoord-nsCoord2)*(nsTarCoord-nsCoord2) + (TarZ-NewZ)*(TarZ-NewZ);
		  
		  if (flDist1 < flDist2)
		  {
			  flDistance = sqrt(flDist1);
		  }
		  else
		  {
			  flDistance = sqrt(flDist2);
		  }
	  }
	  return flDistance;
}

float cPhysics::GetRange ( cLocation usrLoc, cLocation tarLoc )
{
	float nsCoord,ewCoord;
	float nsTarCoord,ewTarCoord;
	float flRange;

	  nsCoord = (((((usrLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flY / 24) - 1027.5; 
      ewCoord = (((((usrLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flX / 24) - 1027.5;
	  nsTarCoord = (((((tarLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(tarLoc.m_flY / 24) - 1027.5; 
      ewTarCoord = (((((tarLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(tarLoc.m_flX / 24) - 1027.5;
	
	  flRange = sqrt(pow(nsTarCoord - nsCoord,2) + pow(ewTarCoord - ewCoord,2));

	 return flRange;

}

float cPhysics::Get3DRange ( cLocation usrLoc, cLocation tarLoc ) //agentsparrow
{
	float nsCoord,ewCoord;
	float nsTarCoord,ewTarCoord;
	float flRange;

	  nsCoord = (((((usrLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flY / 24) - 1027.5; 
      ewCoord = (((((usrLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flX / 24) - 1027.5;
	  nsTarCoord = (((((tarLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(tarLoc.m_flY / 24) - 1027.5; 
      ewTarCoord = (((((tarLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(tarLoc.m_flX / 24) - 1027.5;
	
	  flRange = sqrt(pow(nsTarCoord - nsCoord,2) + pow(ewTarCoord - ewCoord,2) + pow((tarLoc.m_flZ - usrLoc.m_flZ)/24,2));

	 return flRange;
	 
}

cVelocity cPhysics::GetTargetVelocity ( cLocation usrLoc, cLocation tarLoc ) //agentsparrow
{
	float nsCoord,ewCoord;
	float nsTarCoord,ewTarCoord;
	cVelocity tarVel;
	float flRange;

	  nsCoord = (((((usrLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flY / 24) - 1027.5; 
      ewCoord = (((((usrLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flX / 24) - 1027.5;
	  nsTarCoord = (((((tarLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(tarLoc.m_flY / 24) - 1027.5; 
      ewTarCoord = (((((tarLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(tarLoc.m_flX / 24) - 1027.5;
	  
	  flRange = sqrt(pow(nsTarCoord - nsCoord,2) + pow(ewTarCoord - ewCoord,2) + pow((tarLoc.m_flZ - usrLoc.m_flZ)/24,2));

	  tarVel.m_dx = (ewTarCoord - ewCoord)/flRange;
	  tarVel.m_dy = (nsTarCoord - nsCoord)/flRange;
	  tarVel.m_dz = (tarLoc.m_flZ - usrLoc.m_flZ)/24/flRange;

	 return tarVel;

}

cLocation cPhysics::VelocityMove ( cLocation usrLoc, cVelocity tarVel, float flSpeed ) //agentsparrow
{
	float x, y;
	cLocation newLoc;

	newLoc = usrLoc;

	x = (((((usrLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flX / 24) - 1027.5;
	y = (((((usrLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flY / 24) - 1027.5; 

	x += tarVel.m_dx * flSpeed/24;
	x += 1027.5;
	int iLBx = (int) (x/8);
	x -= iLBx*8;
	newLoc.m_flX = x*24;
	iLBx -= 1;
	int squareX = (int) (x+1);
	y += tarVel.m_dy * flSpeed/24;
	y += 1027.5;
	int iLBy = (int) (y/8);
	y -= iLBy*8;
	newLoc.m_flY = y*24;
	iLBy -= 1;
	int squareY = (int) (y+1);
	int square = ((squareX-1)*8)+squareY;
	newLoc.m_dwLandBlock = ((double) iLBx * 0x1000000) + ((double) iLBy * 0x10000) + (double) square;
	newLoc.m_flZ += tarVel.m_dz * flSpeed;

	return newLoc;
}

float cPhysics::GetHeadingTarget( cLocation usrLoc, cLocation tarLoc )
{

	float nsCoord,ewCoord;
	float nsTarCoord,ewTarCoord;
	float flHeading;
	float flRange;

	flHeading = 0.0f;

	nsCoord = (((((usrLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flY / 24) - 1027.5; 
	ewCoord = (((((usrLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(usrLoc.m_flX / 24) - 1027.5;
	nsTarCoord = (((((tarLoc.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(tarLoc.m_flY / 24) - 1027.5; 
	ewTarCoord = (((((tarLoc.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(tarLoc.m_flX / 24) - 1027.5;

	flRange = sqrt(pow(nsTarCoord - nsCoord,2) + pow(ewTarCoord - ewCoord,2));

	if(flRange > 0)
	{
	  if(nsTarCoord - nsCoord < 0 )
	  { 
		  flHeading = acos((ewTarCoord - ewCoord) / flRange) * 57.2957796; 
	  } 
	  else 
	  { 
		  flHeading = acos(-(ewTarCoord - ewCoord) / flRange) * 57.2957796 + 180; 
	  } 
	}
	return flHeading;
}

float cPhysics::GetAvatarHeading( cLocation usrLoc )
{
	float pi = 3.14159f;
	float flHeading;

	if ( (usrLoc.m_flA > 0 && usrLoc.m_flW > 0) || (usrLoc.m_flA < 0 && usrLoc.m_flW > 0) )
		usrLoc.m_flA *= -1;

	flHeading = (360/pi) * acos(usrLoc.m_flA);

	flHeading -= 90;
	
	if (flHeading > 360)
		flHeading -= 360;
	if (flHeading < 0)
		flHeading += 360;

	return flHeading;
}

float cPhysics::GetHeadingDifference( float flUserHeading, float flTargetHeading ) //agentsparrow
{
	float flDifference = 0.0f;

	if ( flUserHeading > flTargetHeading )
	{
		if ( flUserHeading - flTargetHeading < 180 )
		{
			flDifference = flUserHeading - flTargetHeading;
		}
		else
		{
			flDifference = (360 - flUserHeading) + flTargetHeading;
		}
	}
	else if ( flTargetHeading > flUserHeading )
	{
		if ( flTargetHeading - flUserHeading < 180 )
		{
			flDifference = flTargetHeading - flUserHeading;
		}
		else
		{
			flDifference = (360 - flTargetHeading) + flUserHeading;
		}
	}
	return flDifference;
}

float cPhysics::GetLandZ( cLocation usrLoc ) //agentsparrow
{
	float flLandZ;
	float pSW[3], pNW[3], pSE[3], pNE[3];
	float flA, flB, flC;
	int startX, startY, count;
	cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( usrLoc.m_dwLandBlock ) );
	int square = LOBYTE( LOWORD( usrLoc.m_dwLandBlock ) );
	
	flLandZ = usrLoc.m_flZ;

	if (pcLB)
	{
		count = 1;
		startY = square;
		while (square > 8)
		{
			square -= 8;
			startY = square;
			count++;
		}
		startX = count;
		
		pSW[2] = pcLB->m_flLandBlockZ[startX-1][startY-1]*2;
		pNW[2] = pcLB->m_flLandBlockZ[startX-1][startY]*2;
		pSE[2] = pcLB->m_flLandBlockZ[startX][startY-1]*2;
		pNE[2] = pcLB->m_flLandBlockZ[startX][startY]*2;

		pSW[0] = (startX-1)*24;
		pNW[0] = (startX-1)*24;
		pSE[0] = startX*24;
		pNE[0] = startX*24;

		pSW[1] = (startY-1)*24;
		pNW[1] = startY*24;
		pSE[1] = (startY-1)*24;
		pNE[1] = startY*24;

		int x = HIBYTE(pcLB->m_wLandBlock) * 8;
		int y = LOBYTE(pcLB->m_wLandBlock) * 8;
		x += startX - 1;
		y += startY - 1;

		DWORD dw = x * y * 0x0CCAC033 - x * 0x421BE3BD + y * 0x6C1AC587 - 0x519B8F25;
		if ((dw & 0x80000000) != 0) // NE-SW split
		{
			float pNWRange = sqrt(pow(pNW[0] - usrLoc.m_flX,2) + pow(pNW[1] - usrLoc.m_flY,2));
			float pSERange = sqrt(pow(pSE[0] - usrLoc.m_flX,2) + pow(pSE[1] - usrLoc.m_flY,2));
			if ( pNWRange > pSERange ) // usrLoc is closer to pSE
			{
				flA = (pNE[1]-pSE[1])*(pSW[2]-pSE[2])-(pSW[1]-pSE[1])*(pNE[2]-pSE[2]);
				flB = (pNE[2]-pSE[2])*(pSW[0]-pSE[0])-(pSW[2]-pSE[2])*(pNE[0]-pSE[0]);
				flC = (pNE[0]-pSE[0])*(pSW[1]-pSE[1])-(pSW[0]-pSE[0])*(pNE[1]-pSE[1]);

				flLandZ = -flA/flC*(usrLoc.m_flX-pSE[0])-flB/flC*(usrLoc.m_flY-pSE[1])+pSE[2];
			}
			else // usrLoc is closer to pNW
			{
				flA = (pNE[1]-pNW[1])*(pSW[2]-pNW[2])-(pSW[1]-pNW[1])*(pNE[2]-pNW[2]);
				flB = (pNE[2]-pNW[2])*(pSW[0]-pNW[0])-(pSW[2]-pNW[2])*(pNE[0]-pNW[0]);
				flC = (pNE[0]-pNW[0])*(pSW[1]-pNW[1])-(pSW[0]-pNW[0])*(pNE[1]-pNW[1]);

				flLandZ = -flA/flC*(usrLoc.m_flX-pNW[0])-flB/flC*(usrLoc.m_flY-pNW[1])+pNW[2];
			}
		}
		else // NW-SE split
		{
			float pNERange = sqrt(pow(pNE[0] - usrLoc.m_flX,2) + pow(pNE[1] - usrLoc.m_flY,2));
			float pSWRange = sqrt(pow(pSW[0] - usrLoc.m_flX,2) + pow(pSW[1] - usrLoc.m_flY,2));
			if ( pNERange > pSWRange ) // usrLoc is closer to pSW
			{
				flA = (pSE[1]-pSW[1])*(pNW[2]-pSW[2])-(pNW[1]-pSW[1])*(pSE[2]-pSW[2]);
				flB = (pSE[2]-pSW[2])*(pNW[0]-pSW[0])-(pNW[2]-pSW[2])*(pSE[0]-pSW[0]);
				flC = (pSE[0]-pSW[0])*(pNW[1]-pSW[1])-(pNW[0]-pSW[0])*(pSE[1]-pSW[1]);

				flLandZ = -flA/flC*(usrLoc.m_flX-pSW[0])-flB/flC*(usrLoc.m_flY-pSW[1])+pSW[2];
			}
			else // usrLoc is closer to pNE
			{
				flA = (pNW[1]-pNE[1])*(pSE[2]-pNE[2])-(pSE[1]-pNE[1])*(pNW[2]-pNE[2]);
				flB = (pNW[2]-pNE[2])*(pSE[0]-pNE[0])-(pSE[2]-pNE[2])*(pNW[0]-pNE[0]);
				flC = (pNW[0]-pNE[0])*(pSE[1]-pNE[1])-(pSE[0]-pNE[0])*(pNW[1]-pNE[1]);

				flLandZ = -flA/flC*(usrLoc.m_flX-pNE[0])-flB/flC*(usrLoc.m_flY-pNE[1])+pNE[2];
			}
		}
	}

	return flLandZ;
}