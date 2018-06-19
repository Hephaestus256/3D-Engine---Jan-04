#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include <dos.h>
#include <sys\nearptr.h>
#include <sys\farptr.h>
#include <go32.h>
#include <io.h>
#include <dpmi.h>
#include <\programs\include\vesa.h>
#include <\programs\include\fixed.h>

#define FOOT_PER_TEXEL (1/double(15))
#define UNIT_PER_FOOT 5
#define PLAYER_HEIGHT 0
#define CLOCK_GRAN 10000
#define WIDTH 320
#define HEIGHT 200
#define DAT_MID 3000
#define SCAN_SECT 16
#define BYTES_PER_PXL 3
#define byte unsigned char
#define dlong long long

void cleanup_and_exit();
void set_mouse_bound (int x1, int y1, int x2, int y2);
int init_mouse ();
void show_cursor();
void hide_cursor();
void get_mouse_status (short* x, short* y, short* b);
void set_mouse_coord(int x, int y);
void set_vga_mode (int mode);
void pxl (int x, int y, int color);
inline void pxl (point_2d p, int color);
void draw_solid_horz_line (int x1, int x2, int y, int color);
void draw_solid_tri (point_2d p1, point_2d p2, point_2d p3, poly_dat dat, int color);
inline void calc_2d_dxdy_line (line_equat* line, point_2d p1, point_2d p2);
inline void calc_2d_dydx_line (line_equat* line, point_2d p1, point_2d p2);
inline double init_edge (line_equat edge, double ind);
inline double horz_sect_top (double y);
inline double horz_sect_bot (double y);
inline int line_start (double p);
inline int line_end (double p);
inline void draw_horz_first_sect
  (double top, double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline void draw_horz_left_sect
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
void draw_line (point_2d p1, point_2d p2, int color);
inline void draw_horz_right_sect
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline double point_rel_line (point_2d p, line_equat line);
void draw_line (line_equat line, int color);
inline point_2d line_intersect (line_equat line1, line_equat line2);
inline point_2d intersect_vert_line (point_2d point, line_equat);
inline void calc_rot_data (rot_data* dat, double a);
inline void group_rot (rot_data d, double* x, double* y);
inline double simplify_angle (double a);
inline void group_rot
  (rot_data d, double in_x, double in_y, double* out_x, double* out_y);
void calc_rel_3d_points (view_type view, point_type* p, point_type* last);
void init_zoom
  (view_type* view, double center_x, double center_y,
   double field_x, double field_y);
void init_view (view_type* view, double field_x, double field_y);
point_2d map_to_scrn (view_type view, point_type p);
int keyb();
void clear_screen();
void draw_tri_3d (tri_3d tri, poly_dat dat, view_type* view);
inline void simplify_angle (double* a);
inline double deg_to_rad (double deg);
inline bool is_approx_zero (double n);
void edge_clip_left (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view);
point_2d map_to_scrn (view_type view, point_3d p);
inline point_3d intrapolate_3d (point_3d p1, point_3d p2, double z);
inline void edge_clip_right
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view);
inline line_3d calc_line_3d (point_3d p1, point_3d p2);
inline point_3d clip_edge_lr (point_3d p1, point_3d p2, double view_m);
inline point_3d clip_edge_tb (point_3d p1, point_3d p2, double view_m);
inline void edge_clip_top
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view);
inline void edge_clip_bot
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view);
inline void proc_dyn_light
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view);
void write_row_to_scrn (int source, int dest_offset, int len);
void pxl (int x, int y, int blue, int green, int red);
void show_gfx (view_type* view);
void draw_vert_tex_tri
  (point_2d p1, point_2d p2, point_2d p3, poly_dat dat, int color);
inline double vert_sect_top (double x);
inline double vert_sect_bot (double x);
inline void draw_solid_vert_line (int y1, int y2, int x, int color);
inline void draw_vert_first_sect
  (double lef, double rig, double* x,
   line_equat top_edge, line_equat bot_edge, double* t, double* b,
   poly_dat dat, int color, view_type view);
inline void draw_vert_top_sect
  (double rig, double* x,
   line_equat top_edge, line_equat bot_edge, double* t, double* b,
   poly_dat dat, int color, view_type view);
inline void draw_vert_bot_sect
  (double rig, double* x,
   line_equat top_edge, line_equat bot_edge, double* t, double* b,
   poly_dat dat, int color, view_type view);
double inline calc_inf();
void gfx_h_seg1 (int y, int x1, int x2, sng scan_m, int color,
  view_type view);
void gfx_h_seg2 (int x, int y1, int y2, double scan_m, int color,
  view_type view);
void gfx_v_seg1 (int x, int y1, int y2, double scan_m, int color,
  view_type view);
void gfx_v_seg2 (int x, int y1, int y2, double scan_m, int color,
  view_type view);
void gfx_h_line (int y, int x1, int x2, double scan_m, int color,
  view_type view);
void calc_line (line_equat* line, scrn_point* point1, scrn_point* point2);
inline void init_horz_scan (
  double scan_m, double* scan_min, double* scan_max, scrn_point* start,
  line_equat* left, line_equat* right, double* zoom, double* center,
  double* n1, double* n2, double* dn1, double* dn2, double* m3d, double* b3d,
  double* p, double* dp,
  point_type p_far, point_type p_left, point_type p_right, view_type view);
inline void init_vert_scan (
  double scan_m, double* scan_min, double* scan_max, scrn_point* start,
  line_equat* left, line_equat* right, double* zoom, double* center,
  double* n1, double* n2, double* dn1, double* dn2, double* m3d, double* b3d,
  double* p, double* dp,
  point_type p_far, point_type p_left, point_type p_right, view_type view);
inline void init_horz_scan_square (
  double scan_m, double* scan_min, double* scan_max, scrn_point* start,
  line_equat* left, line_equat* right, double* zoom, double* center,
  double* n1, double* n2, double* dn1, double* dn2, double* m3d, double* b3d,
  double* p, double* dp,
  point_type p_far_left, point_type p_near_left,
  point_type p_far_right, point_type p_near_right, view_type view);
inline void init_vert_scan_square (
  double scan_m, double* scan_min, double* scan_max, scrn_point* start,
  line_equat* top, line_equat* bot, double* zoom, double* center,
  double* n1, double* n2, double* dn1, double* dn2, double* m3d, double* b3d,
  double* p, double* dp,
  point_type p_far_top, point_type p_near_top,
  point_type p_far_bot, point_type p_near_bot, view_type view);
inline void find_x_and_dx
  (double* n, double* dn, line_equat edge, double scan_m, double scan_b);
inline void find_y_and_dy
  (double* n, double* dn, line_equat edge, double scan_m, double scan_b);
inline void find_n_and_dn
  (double* n, double* dn, double m1, double b1, double m2, double b2);
inline void find_n_and_dn2
  (double* n, double* dn, double m1, double b1, double m2, double b2);
void calc_bmp_edge
  (line_equat* edge, point_type p_near, point_type p_far, view_type view);
inline bool approx_lesser (double a, double b);
inline bool approx_greater (double a, double b);
inline bool approx_equal (double a, double b);
inline double find_vert_scan_b (point_2d point, double scan_m);
inline double find_horz_scan_b (point_2d point, double scan_m);
inline void calc_point (point_type* point, view_type view);
inline void calc_sect_lr
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_tb
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_bt
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_rl
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_lt
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_tl
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_tr
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_rt
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_br
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_rb
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_bl
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
inline void calc_sect_lb
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp);
int load_bmp (const char* filename, bmp_type* bitmap, int width, int height);
void calc_scan_m
  (double* dx, double* dy, point_type ref1, point_type p1, point_type p2,
   view_type view);
void calc_line (line_equat* line, point_2d point1, point_2d point2);
inline void rotate
  (double* x, double* y, double center_x, double center_y, double angle);
inline double square(double n);
void calc_line_3d (line_3d* l3d, point_3d p1, point_3d p2);
inline double find_horz_scan_b (scrn_point* point, double scan_m);
inline double find_vert_scan_b (scrn_point* point, double scan_m);
void calc_horz_flat_sect
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double lef_x, double rig_x, double lef_dx, double rig_dx,
   double v, double dv,
   double scan, double scan_end, double scan_m);
void calc_vert_flat_sect
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double lef_x, double rig_x, double lef_dx, double rig_dx,
   double v, double dv,
   double scan, double scan_end, double scan_m);
void calc_u_vert_flat_sect
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double lef_x, double rig_x, double lef_dx, double rig_dx,
   double v, double dv,
   double scan, double scan_end, double scan_m);
void calc_u_horz_flat_sect
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double lef_x, double rig_x, double lef_dx, double rig_dx,
   double v, double dv,
   double scan, double scan_end, double scan_m);
inline void rel_to_scr (point_3d rel, scrn_point* scr, view_type view);
inline void rel_to_scr (point_3d rel, point_2d* scr, view_type view);
inline point_3d intrapolate_3d (point_type p1, point_type p2, double z);
double gen_poly
 (scrn_point* start, texdat_type* texdat, bmp_type bmp, point_type* ref,
 view_type view, int* rot);
inline void gen_mult_tbl ();
void show_bmp (bmp_type bmp, int w, int h);
void calc_rel_plane (plane_type* plane, point_type* point);
void calc_abs_plane (plane_type* plane, point_type* point);
void calc_plane
  (plane_type* plane, point_3d point1, point_3d point2, point_3d point3);
int point_relative_to_plane (plane_type plane, point_3d point);
void calc_y_line_3d (y_line_3d* line, point_3d p1, point_3d p2);
inline line_3d calc_aim_line (camera_type ent);
point_3d intersect_line_plane_3d (plane_type plane, line_3d line);
void pxl (point_2d p, int blue, int green, int red);
void gprint (char* s, double v);
unsigned char* load_text();
void printg (char* str, int yc);
inline point_3d calc_light_corner
  (plane_type plane, double x_len, double y_len);
void gprint (char* s, double v, int n);
inline bool is_real_num (double n);
inline bool calc_scrn_edge
  (line_equat* line, point_3d p1, point_3d p2, point_2d* pa, point_2d* pb,
   view_type view);
inline dbl_pair calc_uv_with_z
  (double* zu, double* zv, point_3d ref_orig, point_3d ref_u, point_3d ref_v,
   double z, bmp_type bmp);
inline dbl_pair calc_illum_area
  (point_3d ref_orig, point_3d ref_u, point_3d ref_v, double z,
   double u_len, double v_len, double rad,
   tri_cut* tc, bmp_type bmp, view_type view);
void create_quad (
  point_type* point, point_type* ref,
  double center_x, double center_y, double z,
  double angle_xz, double angle_yz, double angle_xy,
  double obj_width, double obj_height,
  double ref_x, double ref_y, int txl_width, int txl_height);
void init_tex_plane
  (point_3d ref_0, point_3d ref_u, point_3d ref_v, poly_dat* dat,
   int* rot, bmp_type* bmp, view_type view);
inline line_equat calc_3d_dydx_line (point_3d p1, point_3d p2);
inline void draw_vert_first_sect
  (double top, double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline void draw_vert_left_sect
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline void draw_vert_right_sect
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
void update_uv (int scan, int x);
void draw_gfx_v_seg (long start, long array);
void gfx_v_line_pos (int y, int x1, int x2, double scan_m, int color,
  view_type view);
void gfx_v_line_neg (int y, int x1, int x2, double scan_m, int color,
  view_type view);
void update_next_uv (int scan, int x);
inline void draw_vert_first_sect_pos
  (double top, double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline void draw_vert_first_sect_neg
  (double top, double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline void draw_vert_right_sect_pos
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline void draw_vert_right_sect_neg
  (double top, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline void draw_vert_left_sect_pos
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
inline void draw_vert_left_sect_neg
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view);
void calc_bmp_edge
  (line_3d* l3d_m, double* dx, double* dy, point_2d* ref_2d, view_type view);
void draw_abs_line (line_equat line, int r, int g, int b);
void draw_abs_line (line_2d line, int r, int g, int b);
inline bool calc_horz_bmp_edge
  (line_3d l3d, line_2d* ref_2d, double scan_m, view_type view);
inline bool calc_vert_bmp_edge
  (line_3d l3d, line_2d* ref_2d, double scan_m, view_type view);
inline line_3d calc_parll_line_3d (line_3d line, point_3d point);
inline void update_uv (uv_calc_ref* init, texdat_type* dat, int s);
void update_uv ();
void test_funct ();
inline double horz_scan_b (point_2d p, double scan_m);
inline double vert_scan_b (point_2d p, double scan_m);
inline void init_horz_tri
  (point_2d p1, point_2d p2, point_2d p3, uv_calc_ref* uv_init, double scan_m);
inline void init_vert_tri
  (point_2d p1, point_2d p2, point_2d p3, uv_calc_ref* uv_init, double scan_m);
void check_move
  (point_3d curr, point_3d* tenat, double down, double fric, plane_type plane,
   tri_3d tri);
int point_rel_to_plane (plane_type plane, point_3d point, double rad);
inline void offset_point_3d (point_3d* point, double x, double y, double z);
inline point_3d offset_point_3d
  (point_3d point, double x, double y, double z);
inline bool approx_zero (double n);
inline y_line_3d calc_y_line_3d (point_3d p1, point_3d p2);
inline bool is_within_xz_tri (point_3d top, point_3d bot,
   point_3d lef1, point_3d lef2, point_3d rig1, point_3d rig2,
   point_3d point);
inline bool is_within_xz_lef_tri (point_3d top, point_3d mid,
   point_3d bot, point_3d point);
inline bool is_within_xz_rig_tri (point_3d top, point_3d mid,
   point_3d bot, point_3d point);
inline bool is_within_yz_tri (point_3d top, point_3d bot,
   point_3d lef1, point_3d lef2, point_3d rig1, point_3d rig2,
   point_3d point);
inline bool is_within_yz_lef_tri (point_3d top, point_3d mid,
   point_3d bot, point_3d point);
inline bool is_within_yz_rig_tri (point_3d top, point_3d mid,
   point_3d bot, point_3d point);
inline void deflect_xy (point_3d* point, plane_type plane);
inline void deflect_xyz (point_3d* point, plane_type plane);
inline double dz_dx (point_3d p1, point_3d p2);
inline lin_relat calc_dz_dx_line (point_3d p1, point_3d p2);
inline bool is_within_xy_up_tri
  (point_3d top, point_3d bot_lef, point_3d bot_rig, point_3d point);
inline bool is_within_xy_dn_tri
  (point_3d top_lef, point_3d top_rig, point_3d bot, point_3d point);
inline bool is_within_xy_lef_tri
  (point_3d top, point_3d mid, point_3d bot, point_3d point);
inline bool is_within_xy_rig_tri
  (point_3d top, point_3d mid, point_3d bot, point_3d point);
inline bool point_is_in_plane (point_3d point, plane_type plane, tri_3d tri);
bool point_collis_tri
  (point_3d curr, point_3d tenat, plane_type plane, tri_3d tri);
bool point_is_on_tri (point_3d point, plane_type plane, tri_3d tri);
inline void deflect_xy
  (point_3d thru, point_3d* point, double grav, double fric, double rad,
   plane_type plane);
inline void deflect_xyz
  (point_3d thru, point_3d* point, double grav, double fric, plane_type plane);
inline double xyz_plane_fall_m (plane_type plane);
inline double calc_b_offset (double rad, double dx, double dy);
inline lin_relat calc_dxdz_line (point_3d p1, point_3d p2);
inline bool is_within_xz_up_tri (point_3d top,
  point_3d bot_lef, point_3d bot_rig, point_3d point, double rad);
inline bool is_within_xz_dn_tri (point_3d top_lef, point_3d top_rig,
  point_3d bot, point_3d point, double rad);
inline bool is_within_xz_lef_tri (point_3d top, point_3d mid,
  point_3d bot, point_3d point, double rad);
inline bool is_within_xz_rig_tri (point_3d top, point_3d mid,
  point_3d bot, point_3d point, double rad);
inline bool is_within_xz_rect
  (point_3d p1, point_3d p2, double b_lef, double b_rig, double m,
   point_3d point);
inline bool is_within_xz_circ (point_3d cen, double rad, point_3d point);
inline lin_relat calc_dzdx_line (point_3d p1, point_3d p2);
inline void check_move_yz_up_tri
 (point_3d, point_3d, point_3d,
  point_3d pen, point_3d curr, point_3d* tenat,
  double grav, double fric, double rad, plane_type plane);
inline void check_move_yz_dn_tri
 (point_3d, point_3d, point_3d,
  point_3d pen, point_3d curr, point_3d* tenat,
  double grav, double fric, double rad, plane_type plane);
inline void check_move_yz_lef_tri
 (point_3d, point_3d, point_3d,
  point_3d pen, point_3d curr, point_3d* tenat,
  double grav, double fric, double rad, plane_type plane);
inline void check_move_yz_rig_tri
 (point_3d, point_3d, point_3d,
  point_3d pen, point_3d curr, point_3d* tenat,
  double grav, double fric, double rad, plane_type plane);
inline bool pen_yz_plane
  (point_3d curr, point_3d tenat, plane_type plane,
   double radius, point_3d* pen);
inline bool is_in_y_cylinder
  (point_3d p1, point_3d p2, double rad, point_3d point);
inline bool quadratic
  (double a, double b, double c, double*r1, double* r2);
inline double path_int_cyl
  (point_3d inside, point_3d outside, y_line_3d cent, double rad);
inline point_3d path_int_cyl_point
  (point_3d inside, point_3d outside, y_line_3d cent, double rad);
inline point_3d point_perp_line (y_line_3d line, point_3d point);
inline void colis_cylinder
  (y_line_3d center, point_3d top, point_3d bot, double rad,
   point_3d curr, point_3d* tenat, double grav, double fric);
inline double edge_fall_m (point_3d top, point_3d bot);
inline y_line_3d parll_y_line_3d (y_line_3d line, point_3d point);
inline void calc_view (camera_type cursor, view_type* view);
inline void show_cursor (point_3d pos, view_type view);
inline point_2d map_abs_point (point_3d p, view_type view);
void init();
inline void calc_rel_3d_points
  (point_3d center, double angle_xz, double angle_yz, double angle_xy,
   point_type* p, point_type* last);
inline double rad_to_deg (double rad);
inline double units_to_rad (double units);
inline void show_corner (point_type p, view_type view);
void key_init();
void log_key(int k);
void key_handler();
void key_delete();
inline void mouse_button (int raw_dat, int n);
inline void select_point (view_type view, point_type* start, point_type* last);
void update_player
  (camera_type* camera, view_type view,
  point_type* p, point_type** last_point,
  tri_type* tri, tri_type** last_tri);
inline dlong curr_time();
inline x_line_3d calc_x_line_3d (point_3d p1, point_3d p2);
void create_tex_ref (
  tex_ref** last_ref, point_type** last_point,
  point_3d p1, point_3d p2, point_3d p3);
inline void swap_p3d (point_type** p1, point_type** p2);
inline void sort_clockwise
  (point_type** p1, point_type** p2, point_type** p3, view_type view);
inline void create_tri
  (tri_type** last_tri, point_3d* p1, point_3d* p2, point_3d* p3,
  view_type view);

_go32_dpmi_seginfo _old_key_handler, _new_key_handler;
unsigned char* _text;
void* _x2_buff;
texdat_type _texdat[DAT_MID * 2];
long _temp;
unsigned long _f;
unsigned long _scrn;
unsigned long _temp1, _temp2, _temp3, _temp4, _temp5;
unsigned long _temp6, _temp7, _temp8, _temp9, _temp10;
unsigned long _temp11, _temp12, _temp13, _temp14, _temp15;
unsigned long _temp16, _temp17, _temp18, _temp19, _temp20;
unsigned long _temp21, _temp22, _temp23, _temp24, _temp25;
double _dtemp[50];
double _f2sng = 65536.0;
double _half = 0.5;
char _str[500];
bool _zm = false;
unsigned char _mult_tbl[4 * 256][256];
int _seg_mode = 0;
double _scan_m;
int _test_color = 0;
//point_3d _p3d[4];
tri_cut _tc;
dbl_pair _duv;
long _return;
long _esp;
long _ebp;
long _edi;
long _esi;
sng _sng_temp1;
line_2d _test_l2d[10];
//  eax = uv_calc_ref* init, ebx = texdat_type* dat, ecx = int s
uv_calc_ref _uv_init;
int _play_mode = false;
int _snap = 0;
int _rot_xz = 0;
int _rot_yz = 0;
int _rot_xy = 0;
int _cursor_mode = 0;
key_type _keyb_stat[128];
key_type _mouse_btn[2];


struct {
  int count; // 0
  int ind_offset; // 4
  long scan_m; // 8
  int plane_no; // 12
  int back_r; // 16
  int back_g; // 20
  int back_b; // 24
  bmp_type bmp; // 28
} _tri_data;


int main()
{
  view_type view;
  point_type corner[100];
  tri_type tri[100];
  tri_group group[100];
  bmp_type bmp[20];
  point_type* last_point = &corner[6];
  tri_type* last_tri = &tri[1];
  tri_group* last_group;
//  poly_dat dat;

  for (point_type* p = corner; p <= last_point; p++)
    p->select = false;
    
  init();
  init_view (&view, deg_to_rad(55), deg_to_rad(55) / 1.5);

  corner[0].abs.x = -1000; corner[0].abs.y = 0; corner[0].abs.z = 1000;
  corner[1].abs.x = 1000; corner[1].abs.y = 0; corner[1].abs.z = -1000;
  corner[2].abs.x = -1000; corner[2].abs.y = 0; corner[2].abs.z = -1000;
  corner[3].abs.x = 1000; corner[3].abs.y = 0; corner[3].abs.z = 1000;

  corner[4].abs.x = 00; corner[4].abs.y = 0; corner[4].abs.z = 90;
  corner[5].abs.x = 15; corner[5].abs.y = 0; corner[5].abs.z = 90;
//  corner[6].abs.x = 15; corner[6].abs.y = 0; corner[6].abs.z = 75;
  corner[6].abs.x = 00; corner[6].abs.y = 0; corner[6].abs.z = 75;
  
  tri[0].t3d.p1 = &corner[0];
  tri[0].t3d.p2 = &corner[1];
  tri[0].t3d.p3 = &corner[2];
  tri[1].t3d.p1 = &corner[0];
  tri[1].t3d.p2 = &corner[3];
  tri[1].t3d.p3 = &corner[1];

  group[0].ref.orig = &corner[4];
  group[0].ref.u = &corner[5];
  group[0].ref.v = &corner[6];
  group[0].ref.bmp = &bmp[0];
  group[0].first = &tri[0];
  group[0].last = &tri[1];
  group[0].vis_side = -1;
  calc_abs_plane (&group[0].plane, group[0].ref.orig);
//  bmp_type bmp1, bmp2, shad1;
  load_bmp ("bbrick.gfx", &bmp[0], 64, 64);
//  load_bmp ("spot.dat", &shad1, 128, 128);
//  _shad = shad1;

//  dat.ref[0] = &corner[4];
//  dat.ref[1] = &corner[5];
//  dat.ref[2] = &corner[6];
//  dat.ref[2] = &corner[6];
//  _bmp = dat.bmp = bmp1;
  
  corner[8].abs.x = -10;
  corner[8].abs.y = -20;
  corner[8].abs.z = 100;
  
  corner[9].abs.x = -1;
  corner[9].abs.y = -50;
  corner[9].abs.z = 200;

  plane_type plane;
  calc_abs_plane (&plane, corner);
  camera_type cam;
  cam.pos.x = 0;
  cam.pos.y = -50;
  cam.pos.z = 0;
  cam.angle_xz = 0;
  cam.angle_yz = 0;
  cam.angle_xy = 0;
  
  _init_flash = curr_time();
  for (;;) {
    if (curr_time() - _init_flash >= 2000) {
      _init_flash = curr_time();
//      _flash = !_flash;
    }
    double grav = 0;
    if (_play_mode)
      grav = 4;

    calc_view (cam, &view);
    clear_screen();
//    calc_rel_3d_points (view, corner, last_point);
    calc_rel_3d_points (
       view.camera.pos,
       view.camera.angle_xz, view.camera.angle_yz, view.camera.angle_xy,
       corner, last_point);
    calc_rel_plane (&_pl, &corner[0]);

    for (int s = -320 - 200; s <= 320 + 200; s++)
      _texdat[s + DAT_MID].init = 0;

/*
    if (point_relative_to_plane (plane, view.camera.pos) == -1) {
      _bmp = dat.bmp = bmp1;
      draw_tri_3d (tri[0].t3d, dat, &view);
      _bmp = dat.bmp = bmp2;
      draw_tri_3d (tri[1].t3d, dat, &view);
    }
*/

    for (point_type* p = corner; p <= last_point; p++)
      show_corner (*p, view);

    show_cursor (cam.pos, view);

    gprint("%f", cam.pos.x, 0);
    gprint("%f", cam.pos.y, 1);
    gprint("%f", cam.pos.z, 2);
    gprint("%f", _dtemp[10], 23);//rad_to_deg(cam.angle_xz), 23);
    gprint("%f", _dtemp[11], 24);//rad_to_deg(cam.angle_yz), 24);

    show_gfx(&view);
    update_player (&cam, view, corner, &last_point, tri, &last_tri);
    
    for (int i = 0; i < 128; i++)
      _keyb_stat[i].change = false;
  }

  cleanup_and_exit();
}


void init_view (view_type* view, double field_x, double field_y)
{
  _x2_buff = malloc (WIDTH * HEIGHT * 3);
  view->z_map = (z_map_type*)malloc (sizeof(z_map_type) * WIDTH * HEIGHT);
  view->vis_page = 0;
  view->camera.pos.x = 0;
  view->camera.pos.y = -50;
  view->camera.pos.z = 0;
  view->camera.angle_xz = 0;//-PI / 6;//;
  view->camera.angle_yz = 0;//-PI / 6;
  view->camera.angle_xy = 0;
  view->z_cutoff = 1.0001;
  view->center.x = WIDTH / 2;
  view->center.y = HEIGHT / 2;
  view->window[0].x = 0;
  view->window[0].y = 0;
  view->window[1].x = (double)WIDTH;
  view->window[1].y = (double)HEIGHT;
  view->vis_lm = -tan(field_x / 2);
  view->vis_rm = tan(field_x / 2);
  view->vis_tm = -tan(field_y / 2);
  view->vis_bm = tan(field_y / 2);
  init_zoom (view, WIDTH / 2, HEIGHT / 2, field_x / 2, field_y / 2);
}


void init_zoom
  (view_type* view, double center_x, double center_y,
   double field_x, double field_y)
{
  view->zoom.x = center_x / tan(field_x);
  view->zoom.y = center_y / tan(field_y);
}


void update_player
  (camera_type* camera, view_type view,
  point_type* p, point_type** last_point,
  tri_type* tri, tri_type** last_tri)
{
  double rec_inc = 2;

  short mx, my, mb;
  get_mouse_status (&mx, &my, &mb);
  mouse_button (mb, 1);
  mouse_button (mb, 2);
  
  _rot_xz += (mx - 512);
  _rot_xz = (_rot_xz + FULL_ROTATE) % (FULL_ROTATE);
  _rot_yz -= (my - 512);
  _rot_yz = (_rot_yz + FULL_ROTATE) % (FULL_ROTATE);
  set_mouse_coord (512, 512);

  camera->angle_xz = units_to_rad (_rot_xz);
  camera->angle_yz = units_to_rad (_rot_yz);
  camera->angle_xy = units_to_rad (_rot_xy);
  
  int snap_grad = 15;
  
  if (_snap) {
    camera->angle_xz =
      deg_to_rad (snap_grad * int(rad_to_deg (camera->angle_xz) / snap_grad));
    camera->angle_yz =
      deg_to_rad (snap_grad * int(rad_to_deg (camera->angle_yz) / snap_grad));
  }
  
    if (_keyb_stat[0x0C].press)
      _view_rad -= 2.5;
    else if (_keyb_stat[0x0D].press)
      _view_rad += 2.5;
    if (_keyb_stat[0x0F].change)
      if (_cursor_mode) {
        _cursor_mode = false;
        camera->pos.x += sin(camera->angle_xz) * cos(camera->angle_yz) *
          _view_rad;
        camera->pos.y -= sin(camera->angle_yz) * _view_rad;
        camera->pos.z += cos(camera->angle_xz) * cos(camera->angle_yz) *
          _view_rad;
      }
      else {
        _cursor_mode = true;
        camera->pos.x -= sin(camera->angle_xz) * cos(camera->angle_yz) *
          _view_rad;
        camera->pos.y += sin(camera->angle_yz) * _view_rad;
        camera->pos.z -= cos(camera->angle_xz) * cos(camera->angle_yz) *
          _view_rad;
      }
    if (_keyb_stat[0x01].change)
      for (point_type* i = p; i <= *last_point; i++)
        i->select = false;
    if (_keyb_stat[0x2D].change)
      cleanup_and_exit();
    if (_keyb_stat[72].press) {
      camera->pos.x += sin(camera->angle_xz) * cos(camera->angle_yz + PI / 2) * rec_inc;
      camera->pos.y -= sin(camera->angle_yz + PI / 2) * rec_inc;
      camera->pos.z += cos(camera->angle_xz) * cos(camera->angle_yz + PI / 2) * rec_inc;
    }
    if (_keyb_stat[80].press) {
      camera->pos.x -= sin(camera->angle_xz) * cos(camera->angle_yz + PI / 2) * rec_inc;
      camera->pos.y += sin(camera->angle_yz + PI / 2) * rec_inc;
      camera->pos.z -= cos(camera->angle_xz) * cos(camera->angle_yz + PI / 2) * rec_inc;
    }
    if (_keyb_stat[16].press) {
      camera->pos.x += sin(camera->angle_xz) * cos(camera->angle_yz) * rec_inc;
      camera->pos.y -= sin(camera->angle_yz) * rec_inc;
      camera->pos.z += cos(camera->angle_xz) * cos(camera->angle_yz) * rec_inc;
    }
    if (_keyb_stat[30].press) {
      camera->pos.x -= sin(camera->angle_xz) * cos(camera->angle_yz) * rec_inc;
      camera->pos.y += sin(camera->angle_yz) * rec_inc;
      camera->pos.z -= cos(camera->angle_xz) * cos(camera->angle_yz) * rec_inc;
    }
    if (_keyb_stat[3].change) {
      _snap = !_snap;
    }
    if (_keyb_stat[8].press) {
      camera->pos.y -= rec_inc;
    }
    if (_keyb_stat[0x14].change) {
      point_3d sel[3];
      int ct = 0;
      for (point_type* i = p; i <= *last_point; i++)
        if (i->select) {
          sel[ct] = i->abs;
          ct++;
        }
      if (ct == 3)
        ;//create_tex_ref (last_point, last_tri, sel[0], sel[1], sel[2]);
    }
    if (_keyb_stat[2].press) {
      camera->pos.y += rec_inc;
    }
    if (_keyb_stat['K'].press) {
      camera->pos.x -= sin(camera->angle_xz + PI / 2) * rec_inc;
      camera->pos.z -= cos(camera->angle_xz + PI / 2) * rec_inc;
    }
    if (_keyb_stat['M'].press) {
      camera->pos.x += sin(camera->angle_xz + PI / 2) * rec_inc;
      camera->pos.z += cos(camera->angle_xz + PI / 2) * rec_inc;
    }
    if (_mouse_btn[0].change)
      if (_cursor_mode) {
        select_point (view, p, *last_point);
      }
      else {
        (*last_point)++;
        (*last_point)->abs = camera->pos;
        (*last_point)->select = true;
      }
}


inline void gen_mult_tbl ()
{
  for (int a = 0; a < 256 * 4; a++)
    for (int b = 0; b < 256; b++)
      if (((a * b) >> 8) > 255)
        _mult_tbl[a][b] = 255;
      else
        _mult_tbl[a][b] =
        (unsigned char)floor(((double)a * (double)b) / (double)256);
}


void init()
{
  if (init_mouse()) {
    hide_cursor();
    set_mouse_bound (0, 0, 1024, 1024);
    set_mouse_coord (512, 512);
  }
  else {
    printf ("\nCan't init mouse.\n");
    return;
  }

  calc_inf();
  _text = load_text();
  gen_mult_tbl();

  _tri_data.back_r = 128 << 8;
  _tri_data.back_g = 128 << 8;
  _tri_data.back_b = 128 << 8;

  _temp16 = (0 + 128) << 8;
  _temp17 = (0 + 128) << 8;
  _temp18 = (0 + 128) << 8;

  key_init();
  for (int i = 0; i < 128; i++) {
    _keyb_stat[i].press = false;
    _keyb_stat[i].change = false;
  }
  for (int i = 0; i < 2; i++) {
    _mouse_btn[0].press = false;
    _mouse_btn[0].change = false;
  }
  get_vesa_info();
  set_vesa_mode(TRUE_COLOR_320x200);
  set_logical_line_len (WIDTH * 2);
  set_first_pxl (0, 0);
  get_mode_info();
}


/****************************************************************************/

