/*  calc_funcs.h                                                                
 *                                                                              
 *  Header file for calc_funcs.c                                                
 *                                                                              
 *                   Ver 3.2     2002.05.25  Deryck Morales                     
 *                                           <deryck@alumni.carnegiemellon.edu> 
 *                                           Nick Jong                          
 *                                           <nkj@andrew.cmu.edu>               
 */
#ifndef _CALC_FUNCS_H_
#define _CALC_FUNCS_H_

#include <math.h>

#define PAI 3.14159265

int fix_angle(int angle);
int calc_sup_angle(int dir1, int dir2);
int calc_diff_angle(int x, int y);
int calc_middle_angle(int dir1, int dir2);
int deg_to_index_160(int val);
int index_160_to_deg(int idx);
int calc_vec_to_deg(int vx, int vy);

void calc_XY_from_deg_val(int ,int , int *, int *);
void calc_vertical_point_on_line(int px, int py, int lx, int ly, int ldir, int *result_x, int *result_y);
void calc_rotation(int x, int y, int ang, int *nx, int *ny);


#endif

