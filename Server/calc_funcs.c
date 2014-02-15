/*  calc_funcs.c                                                                
 *                                                                              
 *  Calculation Functions                                                       
 *                                                                              
 *                   Ver 1.0     1998.08.25  Keiji nagatani                     
 *                   Ver 3.0     2001.02.13  Ryosuke Sakai                      
 *                                           (rsakai@andrew)                    
 *                   Ver 3.2     2002.05.25  Deryck Morales                     
 *                                           <deryck@alumni.carnegiemellon.edu> 
 *                                           Nick Jong                          
 *                                           <nkj@andrew.cmu.edu>               
 */
#include "calc_funcs.h"


/*************************
 * PRIVATE PROTOTYPES    *
 *************************/
double sin_deg(double value);
double cos_deg(double value);


/*************************
 * PUBLIC FUNCTIONS      *
 *************************/

/*
 * Scales the given angle to its value in the range: [-180,180] degrees.
 * Units for "angle" are in 1/10 degree (so 180 deg = 1800)
 *
 * The scaled value in 1/10 degrees is returned.
 */
int fix_angle(int angle)
{
  while (angle < -1800)
    angle += 3600;
  while (angle > 1800)
    angle -= 3600;
  return(angle);
}

/*
 * Calculates angle between dir1 and dir2 and scales to the range: [1,3599]
 * This calculated value is then returned.
 * Units are in 1/10 degrees.
 */
int calc_sup_angle(int dir1, int dir2)
{
  if ((dir2 - dir1) > 0) return(dir2 - dir1);
  else                   return(dir2 + 3600 - dir1);
}


/*
 * The angle between x and y is scaled to the range: [0,1800]
 * This calculated value is then returned.
 * Units are in 1/10 degrees.
 */
int calc_diff_angle(int x, int y)
{
  int diff;
  
  diff = abs(x - y);
  if (diff > 1800) return(abs(diff - 3600));
  else             return(diff);
}


/*
 * The bisector of angles dir1 and dir2 is calculated and returned.
 * Units are in 1/10 degrees.
 */
int calc_middle_angle(int dir1, int dir2)
{
  if ((dir2 - dir1) > 0){return(fix_angle((dir2 + dir1)/2));}
  else {return(fix_angle((dir2 + 3600 + dir1)/2));}
}


/*
 * Angle "val" is converted to (160 index), which corresponds to the ring of 16 sonar.
 * This index value is then returned.
 * Units are 1/10 degrees and 160 index (1/10th of 16).
 */
int deg_to_index_160(int val)
{
  int idx;
  idx = val * 2 / 45;         /* (idx = val/22.5) */
  if (idx < 0) idx += 160;
  return(idx);
}


/*
 * 160 index value "idx" is converted to 1/10 degrees.
 * This angle is then returned.
 * Units are 1/10 degrees and 160 index. (see deg_to_index_160).
 */
int index_160_to_deg(int idx)
{
  return(fix_angle(idx * 45 / 2));
}


/*
 * Rotate <x,y> ==> <nx,ny> by theta
 * The point (x,y) is rotated about the origin by "ang" degrees.
 * The resultant point is stored to "nx" and "ny" (new x and y)
 */
void calc_rotation(int x, int y, int ang, int *nx, int *ny)
{
  double dx, dy;
  
  dx = (double)x;  dy = (double)y;
  *nx = (int)(dx * cos_deg((double)ang/10.0) - dy*sin_deg((double)ang/10.0));
  *ny = (int)(dx * sin_deg((double)ang/10.0) + dy*cos_deg((double)ang/10.0));
}


/*
 * Convert <degree, value> ==> <x,y>
 * Polar to rectangular conversion.
 *
 *                       ^   \ 
 *                    *y |  /   r = val
 *                       | /    deg = deg
 *                       |/\ 
 *                       +---> *x
 */
void calc_XY_from_deg_val(int deg, int val, int *x, int *y)
{
  double ddeg;
  
  /* calculate radian */
  ddeg = ((double)deg)*PAI/1800.0;
  
  /* calculate distance */
  *x = (int)(cos(ddeg)*(double)val);
  *y = (int)(sin(ddeg)*(double)val);
}


/*
 * Convert <vx, vy> ==> <degree>
 * Given vector coordinates vx,vy, calculate vector angle.
 * This calculated value is returned.
 * Units are 1/10 degrees.
 */
int calc_vec_to_deg(int vx, int vy)
{
  double  deg;
  
  deg = atan2((double)vy, (double)vx)*1800.0 / PAI;
  if (deg < 0.0) deg+=3600.0;
  
  return((int)deg);
}

/* -------------------------------------------
 * calculate the point on line L<lx,ly,ldir>
 * which is vertical to P<px,py>
 * Called by:
 *      ROBOT-FUNCS\trace.c
 *
 *              point P
 *                    |
 *                    |
 *      lineL  ---L---R---------
 * -------------------------------------------
 */
void calc_vertical_point_on_line(int px,        /* (i) point P */
                                 int py,        /* (i) point P */
                                 int lx,        /* (i) point on line L */
                                 int ly,        /* (i) point on line L */
                                 int ldir,      /* (i) deg of line L */
                                 int *result_x, /* (o) the point on line L which is vertical to P */
                                 int *result_y) /* (o) the point on line L which is vertical to P */
{
  int vx, vy, a, b;
  
  /* convert <degree, value> ==> <x,y>
   *       ^   \ 
   *    vy |  /   r = 100
   *       | /    deg = ldir
   *       |/\ 
   *       +---> vx
   */
  calc_XY_from_deg_val(ldir,  /* (i) deg */
		       100,   /* (i) radius */
		       &vx,   /* (o) vx = 100 * cos(ldir) */
		       &vy);  /* (o) vy = 100 * sin(ldir) */
  
  a = vx*px + vy*py;
  b = vy*lx - vx*ly;
  
  *result_x = (vx*a + vy*b ) / 10000;
  *result_y = (vy*a - vx*b ) / 10000;
}


/*************************
 * PRIVATE FUNCTIONS     *
 *************************/

/*
 * Double absolute value
 */
double dabs(double val)
{
  if (val < 0) return(-val);
  else         return( val);
}

/*
 * Cosine where value is in degrees (normally radians)
 */
double cos_deg(double value)
{
  return(cos(value * PAI / 180.0));
}

/*
 *  Sine where value is in degrees (normally radians)
 */
double sin_deg(double value)
{
  return(sin(value * PAI / 180.0));
}

/*
 * Returns 0 if imaginary solutions, otherwise
 * updates x1,x2 with equation roots and returns 1
 */
int calc_quadratic_equation(double a, double b, double c, double *x1, double *x2)
{
  double d;
  
  d = b*b-4*a*c;
  
  if ( d < 0.0) {
    return(0);
  }else {
    *x1 = (-b+sqrt(d))/(2*a);
    *x2 = (-b-sqrt(d))/(2*a);
    
    return(1);
  }
}

/* EOF calc_funcs.c */
/********************************************************************************/