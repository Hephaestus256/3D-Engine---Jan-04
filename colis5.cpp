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

#define WIDTH 320
#define HEIGHT 200
#define CMP_PRECISION (1.0 / 1000000.0)
#define INF (double)1/(double)0
#define DAT_MID 3000
#define PXL_VARI cv_sng(.5)
#define SCAN_SECT 16
#define BYTES_PER_PXL 3
#define WIDTH 320
#define HEIGHT 200
#define FOOT_PER_TEXEL (1/double(15))
#define UNIT_PER_FOOT 5
#define PLAYER_HEIGHT 0
#define _byte_ (unsigned char)

struct true_color_type {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};
  
struct scrn_point {
  double x, y;
  scrn_point* next;
  scrn_point* prev;
};

struct int_pair {
  int x, y;
};

struct dbl_pair {
  double x, y;
};

struct point_2d {
  double x, y;
};

struct point_3d {
  double x, y, z;
};

struct point_type {
  point_3d abs, rel;
  point_2d scr;
};

struct tri_3d {
  point_type* p1;
  point_type* p2;
  point_type* p3;
};

struct line_equat {
  double m, b;
  double inv_m, inv_b;
};

struct lin_relat {
  double m, b;
};

struct line_2d {
  double m, b;
};

struct line_3d {
  double mx, my, bx, by;
};

struct tri_type {
  point_2d p1, p2, p3;
};

struct rot_data {
  double m;
  double x_sub, y_sub;
  double inv_dm;
};

struct dyn_obj_type {
  point_3d curr_pos;
  point_3d prev_pos;
};

struct entity_type {
  point_3d pos;
  double angle_xy;
  double angle_xz;
  double angle_yz;
};

struct z_map_type {
  unsigned long inv_z;
  long poly;
};

struct view_type {
  entity_type camera;
  double z_cutoff;
  int logical_width;
  dbl_pair center;
  dbl_pair zoom;
  dbl_pair window[2];
  line_equat edge[4];
  z_map_type* z_map;
  void* write_to;
  int vis_page;
  rot_data rot_xz, rot_yz, rot_xy;
  double vis_lm, vis_rm, vis_tm, vis_bm;
};

struct bmp_type {
  unsigned char* texel;
  int width, height;
  int w_shift, h_shift;
};

struct texdat_type {
  sng u, v; // 0, 4
  sng du, dv; // 8, 12
  int last_ind; // 16
  double u1, v1; // 20, 28
  double u2, v2; // 36, 44
  double ind1, ind2; // 52, 60
  double inv_len; // 68
  sng next_u, next_v; // 76, 80
  int init; // 84
}; // 88 bytes long

struct poly_dat {
  double scan_m;
  bmp_type bmp;
  point_type* ref[4];
  int rot;
  long jump;
  double s1, s2;
};

struct plane_type {
// z = m1 * x + b1
// b1 = m2 * y + b
  double m1, m2, b;
  double perp_m1, perp_m2;
  double int_den;
  bool y_plane;
  bool m1_inf;
  double sec_m1;
};

class tri_cut {
  inline void disect_tri_b (point_2d p1, point_2d p2, point_2d p3);
  inline void disect_tri_c (point_2d p1, point_2d p2, point_2d p3);
  inline void disect_tri_d (point_2d p1, point_2d p2, point_2d p3);
  inline void store_inside (point_2d p1, point_2d p2, point_2d p3);
  inline void store_outside (point_2d p1, point_2d p2, point_2d p3);
public:
  inline void disect_tri
    (point_2d p1, point_2d p2, point_2d p3, tri_type* i, tri_type* o);
  tri_type* inside;
  tri_type* outside;
  line_equat* line;
  bool edge_exist[4];
  point_2d qa[4], qb[4];
};

struct y_line_3d {
  double mx, mz, bx, bz;
};

struct tri_group {
  tri_3d* first;
  tri_3d* last;
  point_type* ref_0;
  point_type* ref_u;
  point_type* ref_v;
  bmp_type* bmp;  
};

struct uv_calc_ref {
  int jump;
  double s1, s2;
  line_2d edge1, edge2;
  lin_relat ref_2d;
  lin_relat ref_3d;
  double zoom;
  double center;
  double inv_diff_1;
  double inv_diff_2;
  double orig;
  bmp_type* bmp;
};

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
void update_player (view_type* view);
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
inline line_3d calc_aim_line (entity_type ent);
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
void update_uv (uv_calc_ref* init, texdat_type* dat, int s);
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

plane_type _pl;
double _test[50];
point_3d _test_p3d[10];
unsigned char* _text;
void* _x2_buff;
double _inf_double;
float _inf_float;
double _nan_double;
float _nan_float;
bool _texed = true;
texdat_type _texdat[DAT_MID * 2];
bmp_type _bmp, _shad;
short _poly;
long _temp;
long _ct;
long _o;
unsigned long _f;
sng _a, _b;
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
  bmp_type bmp1, bmp2, shad1;
  calc_inf();
  _text = load_text();
  gen_mult_tbl();
  point_3d prev_cam;
  
  _temp16 = (0 + 155) << 8;
  _temp17 = (0 + 155) << 8;
  _temp18 = (0 + 155) << 8;
  point_2d p1, p2, p3;
  point_type corner[100];
  tri_3d t3d[100];
  tri_cut tc;
  point_type* last_point = &corner[9];
  view_type view;
  
  get_vesa_info();
  set_vesa_mode(TRUE_COLOR_320x200);
  set_logical_line_len (WIDTH * 2);
  set_first_pxl (0, 0);
  get_mode_info();
  _x2_buff = malloc (WIDTH * HEIGHT * 3);
  view.z_map = (z_map_type*)malloc (sizeof(z_map_type) * WIDTH * HEIGHT);
  view.vis_page = 0;
  init_view (&view, deg_to_rad(60), (200 / 320.0) * deg_to_rad(60));
  poly_dat dat;
  point_3d curr_feet, tenat_feet;
  
  corner[0].abs.x = 0; corner[0].abs.y = 10; corner[0].abs.z = 90;
  corner[1].abs.x = 80; corner[1].abs.y = 10; corner[1].abs.z = 10;
  corner[2].abs.x = 0; corner[2].abs.y = 10; corner[2].abs.z = 10;
  corner[3].abs.x = 80; corner[3].abs.y = 10; corner[3].abs.z = 90;

  corner[4].abs.x = 00; corner[4].abs.y = 10; corner[4].abs.z = 90;
  corner[5].abs.x = 15; corner[5].abs.y = 10; corner[5].abs.z = 90;
  corner[6].abs.x = 15; corner[6].abs.y = 10; corner[6].abs.z = 75;
  corner[7].abs.x = 00; corner[7].abs.y = 10; corner[7].abs.z = 75;
  
  t3d[0].p1 = &corner[0];
  t3d[0].p2 = &corner[1];
  t3d[0].p3 = &corner[2];
  t3d[1].p1 = &corner[0];
  t3d[1].p2 = &corner[3];
  t3d[1].p3 = &corner[1];
  
  load_bmp ("grid.gfx", &bmp1, 64, 64);
  load_bmp ("grid.gfx", &bmp2, 64, 64);
  load_bmp ("spot.dat", &shad1, 128, 128);
  _shad = shad1;

  dat.ref[0] = &corner[4];
  dat.ref[1] = &corner[5];
  dat.ref[2] = &corner[6];
  dat.ref[3] = &corner[7];
  _bmp = dat.bmp = bmp1;
  
  corner[8].abs.x = -10;
  corner[8].abs.y = -20;
  corner[8].abs.z = 100;
  
  corner[9].abs.x = -1;
  corner[9].abs.y = -50;
  corner[9].abs.z = 200;

  long long it = uclock();

  _dtemp[25] = -1;
  _dtemp[26] = 0;

  for (int i = 0; i < 8; i++)
    rotate (&corner[i].abs.x, &corner[i].abs.y, 40, 10, -PI / 4);
//  for (int i = 0; i < 8; i++)
//    rotate (&corner[i].abs.x, &corner[i].abs.z, 40, 10, PI / 6);

  plane_type plane;
  calc_abs_plane (&plane, corner);

  for (;;) {
    double grav = 0;
    if (_play_mode)
      grav = 4;

    _dtemp[20] = 1000000;
    _dtemp[21] = -1000000;
    _dtemp[22] = 0;
    _test[0] = 0;
    _test[1] = 0;
    clear_screen();
    calc_rel_3d_points (view, corner, last_point);
    calc_rel_plane (&_pl, &corner[0]);
//    calc_illum_area (&_tc, _pl, view);

    double u, v;
    point_3d zua, zub, zva, zvb;
//    _duv = calc_uv_with_z
//      (&u, &v, corner[4 + 0].rel, corner[4 + 1].rel, corner[4 + 3].rel, _pl.b, _bmp);
//    point_3d ill[4];
    _duv = calc_illum_area
      (corner[4 + 0].rel, corner[4 + 1].rel, corner[4 + 3].rel, _pl.b,
      15.0, 15.0, 15.0, &_tc, _bmp, view);
//inline dbl_pair calc_illum_area
//  (point_3d ref_orig, point_3d ref_u, point_3d ref_v, double z,
//   double u_len, double v_len, double rad,
//   tri_cut* tc, bmp_type bmp, view_type view)

//    calc_illum_area
//      (point_3d ref_orig, point_3d ref_u, point_3d ref_v,
//       double u_len, double v_len, double rad, double z;
//       tri_cut* tc, view_type view);

//    calc_illum_area (
//      corner[4 + 0].rel, corner[4 + 1].rel, corner[4 + 3].rel,
//      15.0, 15.0, 15.0, _pl.b, _bmp, &_tc, view);
//    calc_illum_area

//   double int_zu, double int_zv,
//   tri_cut* tc, view_type view)

    for (int s = -320 - 200; s <= 320 + 200; s++)
      _texdat[s + DAT_MID].init = 0;

    if (point_relative_to_plane (plane, view.camera.pos) == -1) {
//      _bmp = dat.bmp = bmp1;
//      draw_tri_3d (t3d[0], dat, &view);
      _bmp = dat.bmp = bmp2;
      draw_tri_3d (t3d[1], dat, &view);
    }
    double fps = (((double)uclock() - it) / (double)UCLOCKS_PER_SEC);
    it = uclock();

    point_3d curr = view.camera.pos;
    update_player (&view);
    view.camera.pos.y += grav;
//    check_move (curr, &view.camera.pos, grav, .35, plane, t3d[0]);
    check_move (curr, &view.camera.pos, grav, .95, plane, t3d[1]);

//    gprint ("%f", _dtemp[29], 0);
    gprint ("%f", view.camera.pos.x, 1);
    gprint ("%f", view.camera.pos.z, 2);
//    gprint ("%f", _test[22], 1);

//    gprint ("%f", view.camera.pos.y, 1);
    show_gfx(&view);
//void check_move (dyn_obj_type point, plane_type plane)
//    view.camera.angle_xz -= .05;
  }
}


void draw_horz_tex_tri
  (point_2d p1, point_2d p2, point_2d p3, poly_dat dat, int color,
   view_type view)
{
  double y;
  double lef, rig;
  line_equat line_12, line_23, line_31;
  
  if (p1.y < p2.y)
    if (p3.y < p1.y) {
      calc_2d_dxdy_line (&line_12, p1, p2);
      calc_2d_dxdy_line (&line_23, p2, p3);
      calc_2d_dxdy_line (&line_31, p3, p1);
      draw_horz_first_sect (p3.y, p1.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
      draw_horz_right_sect (p2.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
//      printf ("3, 1, 2");
    }
    else if (p3.y > p1.y)
      if (p3.y > p2.y) {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_horz_first_sect (p1.y, p2.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
        draw_horz_right_sect (p3.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
//        printf ("1, 2, 3");
      }
      else if (p3.y < p2.y) {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_horz_first_sect (p1.y, p3.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
        draw_horz_left_sect (p2.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
//        printf ("1, 3, 2");
      }
      else {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_horz_first_sect (p1.y, p2.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
//        printf ("1, 2-3");
      }
    else {
      calc_2d_dxdy_line (&line_12, p1, p2);
      calc_2d_dxdy_line (&line_23, p2, p3);
      draw_horz_first_sect (p1.y, p2.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
//      printf ("1-3, 2");
    }
  else if (p1.y > p2.y)
    if (p3.y < p2.y) {
      calc_2d_dxdy_line (&line_12, p1, p2);
      calc_2d_dxdy_line (&line_23, p2, p3);
      calc_2d_dxdy_line (&line_31, p3, p1);
      draw_horz_first_sect (p3.y, p2.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
      draw_horz_left_sect (p1.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
//      printf ("3, 2, 1");
    }
    else if (p3.y > p2.y)
      if (p3.y > p1.y) {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_horz_first_sect (p2.y, p1.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
        draw_horz_left_sect (p3.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
//        printf ("2, 1, 3");
      }
      else if (p3.y < p1.y) {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_horz_first_sect (p2.y, p3.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
        draw_horz_right_sect (p1.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
//        printf ("2, 3, 1");
      }
      else {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        draw_horz_first_sect (p2.y, p3.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
//        printf ("2, 1-3");
      }
    else {
      calc_2d_dxdy_line (&line_12, p1, p2);
      calc_2d_dxdy_line (&line_31, p3, p1);
      draw_horz_first_sect (p2.y, p1.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
//      printf ("2-3, 1");
    }
  else
    if (p3.y < p1.y) {
      calc_2d_dxdy_line (&line_23, p2, p3);
      calc_2d_dxdy_line (&line_31, p3, p1);
      draw_horz_first_sect (p3.y, p1.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
//      printf ("3, 1-2");
    }
    else if (p3.y > p1.y) {
      calc_2d_dxdy_line (&line_23, p2, p3);
      calc_2d_dxdy_line (&line_31, p3, p1);
      draw_horz_first_sect (p1.y, p3.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
//      printf ("1-2, 3");
    }
}


void draw_vert_tex_tri
  (point_2d p1, point_2d p2, point_2d p3, poly_dat dat, int color,
   view_type view)
{
  double y;
  double lef, rig;
  line_equat line_12, line_23, line_31;
  
  if (dat.scan_m > 0)
    if (p1.y < p2.y)
      if (p3.y < p1.y) {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_pos (p3.y, p1.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
        draw_vert_right_sect_pos (p2.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
//      printf ("3, 1, 2");
      }
      else if (p3.y > p1.y)
        if (p3.y > p2.y) {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_pos (p1.y, p2.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
          draw_vert_right_sect_pos (p3.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
//        printf ("1, 2, 3");
        }
        else if (p3.y < p2.y) {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_pos (p1.y, p3.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
          draw_vert_left_sect_pos (p2.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
//        printf ("1, 3, 2");
        }
        else {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_pos (p1.y, p2.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
//        printf ("1, 2-3");
        }
      else {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        draw_vert_first_sect_pos (p1.y, p2.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
//      printf ("1-3, 2");
      }
    else if (p1.y > p2.y)
      if (p3.y < p2.y) {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_pos (p3.y, p2.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
        draw_vert_left_sect_pos (p1.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
//      printf ("3, 2, 1");
      }
      else if (p3.y > p2.y)
        if (p3.y > p1.y) {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_pos (p2.y, p1.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
          draw_vert_left_sect_pos (p3.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
//        printf ("2, 1, 3");
        }
        else if (p3.y < p1.y) {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_pos (p2.y, p3.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
          draw_vert_right_sect_pos (p1.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
//        printf ("2, 3, 1");
        }
        else {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          draw_vert_first_sect_pos (p2.y, p3.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
//        printf ("2, 1-3");
        }
      else {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_pos (p2.y, p1.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
//      printf ("2-3, 1");
      }
    else
      if (p3.y < p1.y) {
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_pos (p3.y, p1.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
//      printf ("3, 1-2");
      }
      else if (p3.y > p1.y) {
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_pos (p1.y, p3.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
//      printf ("1-2, 3");
      }
      else
        ;
  else
    if (p1.y > p2.y)
      if (p3.y > p1.y) {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_neg (p3.y, p1.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
        draw_vert_left_sect_neg (p2.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
//      printf ("3, 1, 2");
      }
      else if (p3.y < p1.y)
        if (p3.y < p2.y) {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_neg (p1.y, p2.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
          draw_vert_left_sect_neg (p3.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
//        printf ("1, 2, 3");
        }
        else if (p3.y > p2.y) {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_neg (p1.y, p3.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
          draw_vert_right_sect_neg (p2.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
//        printf ("1, 3, 2");
        }
        else {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_neg (p1.y, p2.y, &y, line_12, line_31, &lef, &rig, dat, color, view);
//        printf ("1, 2-3");
        }
      else {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        draw_vert_first_sect_neg (p1.y, p2.y, &y, line_12, line_23, &lef, &rig, dat, color, view);
//      printf ("1-3, 2");
      }
    else if (p1.y < p2.y)
      if (p3.y > p2.y) {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_neg (p3.y, p2.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
        draw_vert_right_sect_neg (p1.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
//      printf ("3, 2, 1");
      }
      else if (p3.y < p2.y)
        if (p3.y < p1.y) {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_neg (p2.y, p1.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
          draw_vert_right_sect_neg (p3.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
//        printf ("2, 1, 3");
        }
        else if (p3.y > p1.y) {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          calc_2d_dxdy_line (&line_31, p3, p1);
          draw_vert_first_sect_neg (p2.y, p3.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
          draw_vert_left_sect_neg (p1.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
//        printf ("2, 3, 1");
        }
        else {
          calc_2d_dxdy_line (&line_12, p1, p2);
          calc_2d_dxdy_line (&line_23, p2, p3);
          draw_vert_first_sect_neg (p2.y, p3.y, &y, line_23, line_12, &lef, &rig, dat, color, view);
//        printf ("2, 1-3");
        }
      else {
        calc_2d_dxdy_line (&line_12, p1, p2);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_neg (p2.y, p1.y, &y, line_31, line_12, &lef, &rig, dat, color, view);
//      printf ("2-3, 1");
      }
    else
      if (p3.y > p1.y) {
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_neg (p3.y, p1.y, &y, line_31, line_23, &lef, &rig, dat, color, view);
//      printf ("3, 1-2");
      }
      else if (p3.y < p1.y) {
        calc_2d_dxdy_line (&line_23, p2, p3);
        calc_2d_dxdy_line (&line_31, p3, p1);
        draw_vert_first_sect_neg (p1.y, p3.y, &y, line_23, line_31, &lef, &rig, dat, color, view);
//      printf ("1-2, 3");
      }
}


inline void draw_solid_horz_line (int x1, int x2, int y, int color)
{
  for (int x = x1; x <= x2; x++)
    pxl (x, y, color, color, color);
}


inline void draw_solid_vert_line (int y1, int y2, int x, int color)
{
  for (int y = y1; y <= y2; y++)
    pxl (x, y, color, color, color);
}


inline void pxl (int x, int y, int color)
{
 if ((x >= 0) & (y >= 0) & (x <= 319) & (y <= 199))
   _farpokeb (_dos_ds, 0xA0000 + x + y * 320, color);
 else {
   printf ("pxl out of bounds %i, %i\n", x, y);
   getch();
 }
}


inline void pxl (point_2d p, int color)
{
 if (int(p.x) >= 0)
   if (int(p.y) >= 0)
     if (int(p.x) <= 319)
       if (int(p.y) <= 199)
         _farpokeb (_dos_ds, 0xA0000 + int(p.x) + int(p.y) * 320, color);
}


inline void calc_2d_dxdy_line
  (line_equat* line, point_2d p1, point_2d p2)
{
  line->m = (p2.x - p1.x) / (p2.y - p1.y);
  line->b = p1.x - p1.y * line->m;
}


inline void calc_2d_dydx_line
  (line_equat* line, point_2d p1, point_2d p2)
{
  line->m = (p2.y - p1.y) / (p2.x - p1.x);

  if (fabs(line->m) == _inf_double)
    line->b = p1.x;
  else
    line->b = p1.y - p1.x * line->m;


/*
  if (p1.x == p2.x) {
    line->m = _inf_double;
    line->b = p1.x;
  }
  else
  {
    line->m = (p2.y - p1.y) / (p2.x - p1.x);
    line->b = p1.y - p1.x * line->m;
  }
*/
}


inline double init_edge (line_equat edge, double ind)
{
  return edge.m * ind + edge.b;
}


inline double horz_sect_top (double y)
{
  return floor(y + .5) + .5;
}


inline double horz_sect_bot (double y)
{
  return floor(y - .5) + .5;
}


inline double vert_sect_top (double x)
{
  return floor(x + .5) + .5;
}


inline double vert_sect_bot (double x)
{
  return floor(x - .5) + .5;
}


inline int line_start (double p)
{
  return int(floor(p + .5) + .5);
}


inline int line_end (double p)
{
  return int(floor(p - .5) + .5);
}


inline void draw_horz_first_sect
  (double top, double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *y = horz_sect_top (top);
  *l = init_edge (left_edge, *y);
  *r = init_edge (right_edge, *y);

  for (; *y <= horz_sect_bot (bot); (*y)++) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_h_line ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l += left_edge.m;
    *r += right_edge.m;
  }
}


inline void draw_horz_left_sect
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *l = init_edge (left_edge, *y);

  for (; *y <= horz_sect_bot (bot); (*y)++) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_h_line ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l += left_edge.m;
    *r += right_edge.m;
  }
}


inline void draw_horz_right_sect
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *r = init_edge (right_edge, *y);

  for (; *y <= horz_sect_bot (bot); (*y)++) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_h_line ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l += left_edge.m;
    *r += right_edge.m;
  }
}


inline void draw_vert_first_sect_neg
  (double bot, double top, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *y = vert_sect_bot (bot);
  *l = init_edge (left_edge, *y);
  *r = init_edge (right_edge, *y);

  for (; *y >= vert_sect_top (top); (*y)--) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_v_line_neg ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l -= left_edge.m;
    *r -= right_edge.m;
  }
}


inline void draw_vert_first_sect_pos
  (double top, double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *y = vert_sect_top (top);
  *l = init_edge (left_edge, *y);
  *r = init_edge (right_edge, *y);

  for (; *y <= vert_sect_bot (bot); (*y)++) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_v_line_pos ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l += left_edge.m;
    *r += right_edge.m;
  }
}


inline void draw_vert_left_sect_neg
  (double top, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *l = init_edge (left_edge, *y);

  for (; *y >= vert_sect_top (top); (*y)--) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_v_line_neg ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l -= left_edge.m;
    *r -= right_edge.m;
  }
}


inline void draw_vert_left_sect_pos
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *l = init_edge (left_edge, *y);

  for (; *y <= vert_sect_bot (bot); (*y)++) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_v_line_pos ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l += left_edge.m;
    *r += right_edge.m;
  }
}


inline void draw_vert_right_sect_neg
  (double top, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *r = init_edge (right_edge, *y);

  for (; *y >= vert_sect_top (top); (*y)--) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_v_line_neg ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l -= left_edge.m;
    *r -= right_edge.m;
  }
}


inline void draw_vert_right_sect_pos
  (double bot, double* y,
   line_equat left_edge, line_equat right_edge, double* l, double* r,
   poly_dat dat, int color, view_type view)
{
  *r = init_edge (right_edge, *y);

  for (; *y <= vert_sect_bot (bot); (*y)++) {
//    draw_solid_horz_line (line_start(*l), line_end(*r), (int)*y, color);
    gfx_v_line_pos ((int)*y, line_start(*l), line_end(*r), dat.scan_m, color, view);
    *l += left_edge.m;
    *r += right_edge.m;
  }
}


inline double point_rel_line (point_2d p, line_equat line)
{
  return p.y - (line.m * p.x + line.b);
}



inline void tri_cut::disect_tri
  (point_2d p1, point_2d p2, point_2d p3, tri_type* i, tri_type* o)
{
  inside = i; outside = o;
  line_equat line_12, line_23, line_31;

  calc_2d_dydx_line (&line_12, p1, p2);
  calc_2d_dydx_line (&line_23, p2, p3);
  calc_2d_dydx_line (&line_31, p3, p1);

  int color1 = 4, color2 = 9;
//  disect_tri_b (p1, p2, p3);
//  _test_color = 0;
//  return;
  if (!edge_exist[0]) {
    disect_tri_b (p1, p2, p3);
    return;
  }
  
//  if (qa[0].x < qb[0].x)
  if (approx_equal (qa[0].x, qb[0].x))
    if (qa[0].y < qb[0].y)
      if (p1.x < qa[0].x)
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x)
//            store_outside (p1, p2, p3);
            disect_tri_b (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_31);
            disect_tri_b (p1, p2, pb);
            disect_tri_b (pa, pb, p2);
            store_outside (pa, p3, pb);
          }
        else
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_12);
            disect_tri_b (p1, pa, p3);
            disect_tri_b (p1, pb, pa);
            store_outside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_31);
            pb = intersect_vert_line (qa[0], line_12);
            disect_tri_b (p1, pb, pa);
            store_outside (pa, pb, p3);
            store_outside (p3, pb, p2);
          }
      else
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_12);
            pb = intersect_vert_line (qa[0], line_31);
            disect_tri_b (p2, p3, pb);
            disect_tri_b (p2, pb, pa);
            store_outside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_12);
            pb = intersect_vert_line (qa[0], line_23);
            disect_tri_b (p2, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pb, p3, p1);
          }
        else
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_31);
            disect_tri_b (p3, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pa, p1, p2);
          }
          else
//            disect_tri_b (p1, p2, p3);
            store_outside (p1, p2, p3);
    else
      if (p1.x < qa[0].x)
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x)
//            disect_tri_b (p1, p2, p3);
            store_outside (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_31);
            store_outside (p1, p2, pb);
            store_outside (pa, pb, p2);
            disect_tri_b (pa, p3, pb);
          }
        else
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_12);
            store_outside (p1, pa, p3);
            store_outside (p1, pb, pa);
            disect_tri_b (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_31);
            pb = intersect_vert_line (qa[0], line_12);
            store_outside (p1, pb, pa);
            disect_tri_b (pa, pb, p3);
            disect_tri_b (p3, pb, p2);
          }
      else
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_12);
            pb = intersect_vert_line (qa[0], line_31);
            store_outside (p2, p3, pb);
            store_outside (p2, pb, pa);
            disect_tri_b (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_12);
            pb = intersect_vert_line (qa[0], line_23);
            store_outside (p2, pb, pa);
            disect_tri_b (pa, pb, p1);
            disect_tri_b (pb, p3, p1);
          }
        else
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_31);
            store_outside (p3, pb, pa);
            disect_tri_b (pa, pb, p1);
            disect_tri_b (pa, p1, p2);
          }
          else
            disect_tri_b (p1, p2, p3);
  else

  if (qa[0].x < qb[0].x)
    if (point_rel_line (p1, line[0]) < 0)
      if (point_rel_line (p2, line[0]) < 0)
        if (point_rel_line (p3, line[0]) < 0)
          store_outside (p1, p2, p3); // all less
        else {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_31);
          pb = line_intersect (line[0], line_23);
          store_outside (p1, p2, pa);
          store_outside (pa, p2, pb);
          disect_tri_b (pa, pb, p3);
        }
      else
        if (point_rel_line (p3, line[0]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_23);
          pb = line_intersect (line[0], line_12);
          store_outside (p1, pa, p3);
          store_outside (p1, pb, pa);
          disect_tri_b (pa, pb, p2);
        }
        else {
          point_2d pa, pb; // ***
          pa = line_intersect (line[0], line_31);
          pb = line_intersect (line[0], line_12);
          store_outside (p1, pb, pa);
          disect_tri_b (pa, pb, p3);
          disect_tri_b (p3, pb, p2);
        }
    else
      if (point_rel_line (p2, line[0]) < 0)
        if (point_rel_line (p3, line[0]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_12);
          pb = line_intersect (line[0], line_31);
          store_outside (p2, p3, pb);
          store_outside (p2, pb, pa);
          disect_tri_b (pa, pb, p1);
        }
        else {
          point_2d pa, pb; // *
          pa = line_intersect (line[0], line_12);
          pb = line_intersect (line[0], line_23);
          store_outside (p2, pb, pa);
          disect_tri_b (pa, pb, p1);
          disect_tri_b (pb, p3, p1);
        }
      else
        if (point_rel_line (p3, line[0]) < 0) {
          point_2d pa, pb; // *
          pa = line_intersect (line[0], line_23);
          pb = line_intersect (line[0], line_31);
          store_outside (p3, pb, pa);
          disect_tri_b (pa, pb, p1);
          disect_tri_b (pa, p1, p2);
        }
        else
          disect_tri_b (p1, p2, p3);
  else if (qa[0].x > qb[0].x)
    if (point_rel_line (p1, line[0]) < 0)
      if (point_rel_line (p2, line[0]) < 0)
        if (point_rel_line (p3, line[0]) < 0)
//          store_outside (p1, p2, p3);
          disect_tri_b (p1, p2, p3);
        else {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_31);
          pb = line_intersect (line[0], line_23);
          disect_tri_b (p1, p2, pa);
          disect_tri_b (pa, p2, pb);
          store_outside (pa, pb, p3);
        }
      else
        if (point_rel_line (p3, line[0]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_23);
          pb = line_intersect (line[0], line_12);
          disect_tri_b (p1, pa, p3);
          disect_tri_b (p1, pb, pa);
          store_outside (pa, pb, p2);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_31);
          pb = line_intersect (line[0], line_12);
          disect_tri_b (p1, pb, pa);
          store_outside (pa, pb, p3);
          store_outside (p3, pb, p2);
        }
    else
      if (point_rel_line (p2, line[0]) < 0)
        if (point_rel_line (p3, line[0]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_12);
          pb = line_intersect (line[0], line_31);
          disect_tri_b (p2, p3, pb);
          disect_tri_b (p2, pb, pa);
          store_outside (pa, pb, p1);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_12);
          pb = line_intersect (line[0], line_23);
          disect_tri_b (p2, pb, pa);
          store_outside (pa, pb, p1);
          store_outside (pb, p3, p1);
        }
      else
        if (point_rel_line (p3, line[0]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[0], line_23);
          pb = line_intersect (line[0], line_31);
          disect_tri_b (p3, pb, pa);
          store_outside (pa, pb, p1);
          store_outside (pa, p1, p2);
        }
        else
          store_outside (p1, p2, p3);
  else
    if (qa[0].y < qb[0].y)
      if (p1.x < qa[0].x)
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x)
//            store_outside (p1, p2, p3);
            disect_tri_b (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_31);
            disect_tri_b (p1, p2, pb);
            disect_tri_b (pa, pb, p2);
            store_outside (pa, p3, pb);
          }
        else
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_12);
            disect_tri_b (p1, pa, p3);
            disect_tri_b (p1, pb, pa);
            store_outside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_31);
            pb = intersect_vert_line (qa[0], line_12);
            disect_tri_b (p1, pb, pa);
            store_outside (pa, pb, p3);
            store_outside (p3, pb, p2);
          }
      else
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_12);
            pb = intersect_vert_line (qa[0], line_31);
            disect_tri_b (p2, p3, pb);
            disect_tri_b (p2, pb, pa);
            store_outside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_12);
            pb = intersect_vert_line (qa[0], line_23);
            disect_tri_b (p2, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pb, p3, p1);
          }
        else
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_31);
            disect_tri_b (p3, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pa, p1, p2);
          }
          else
//            disect_tri_b (p1, p2, p3);
            store_outside (p1, p2, p3);
    else
      if (p1.x < qa[0].x)
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x)
//            disect_tri_b (p1, p2, p3);
            store_outside (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_31);
            store_outside (p1, p2, pb);
            store_outside (pa, pb, p2);
            disect_tri_b (pa, p3, pb);
          }
        else
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_12);
            store_outside (p1, pa, p3);
            store_outside (p1, pb, pa);
            disect_tri_b (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_31);
            pb = intersect_vert_line (qa[0], line_12);
            store_outside (p1, pb, pa);
            disect_tri_b (pa, pb, p3);
            disect_tri_b (p3, pb, p2);
          }
      else
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_12);
            pb = intersect_vert_line (qa[0], line_31);
            store_outside (p2, p3, pb);
            store_outside (p2, pb, pa);
            disect_tri_b (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_12);
            pb = intersect_vert_line (qa[0], line_23);
            store_outside (p2, pb, pa);
            disect_tri_b (pa, pb, p1);
            disect_tri_b (pb, p3, p1);
          }
        else
          if (p3.x < qa[0].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[0], line_23);
            pb = intersect_vert_line (qa[0], line_31);
            store_outside (p3, pb, pa);
            disect_tri_b (pa, pb, p1);
            disect_tri_b (pa, p1, p2);
          }
          else
            disect_tri_b (p1, p2, p3);
}


inline void tri_cut::disect_tri_b (point_2d p1, point_2d p2, point_2d p3)
{
//  store_outside (p1, p2, p3, 15);
//  return;
//  store_inside (p1, p2, p3);
//  return;
  
  line_equat line_12, line_23, line_31;

  calc_2d_dydx_line (&line_12, p1, p2);
  calc_2d_dydx_line (&line_23, p2, p3);
  calc_2d_dydx_line (&line_31, p3, p1);

  int color1 = 4, color2 = 9;
  
//  printf ("<%f>", point_rel_line (p1, line[1]));
//  printf ("<%f>", point_rel_line (p2, line[1]));
//  printf ("<%f>", point_rel_line (p3, line[1]));
  
  if (!edge_exist[1]) {
    disect_tri_c (p1, p2, p3);
    return;
  }

  if (approx_equal (qa[1].x, qb[1].x))
    if (qa[1].y < qb[1].y)
      if (p1.x < qa[1].x)
        if (p2.x < qa[1].x)
          if (p3.x < qa[1].x)
            disect_tri_c  (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_31);
            disect_tri_c (p1, p2, pb);
            disect_tri_c (pa, pb, p2);
            store_outside (pa, p3, pb);
          }
        else
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_12);
            disect_tri_c (p1, pa, p3);
            disect_tri_c (p1, pb, pa);
            store_outside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_31);
            pb = intersect_vert_line (qa[1], line_12);
            disect_tri_c (p1, pb, pa);
            store_outside (pa, pb, p3);
            store_outside (p3, pb, p2);
          }
      else
        if (p2.x < qa[1].x)
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_12);
            pb = intersect_vert_line (qa[1], line_31);
            disect_tri_c (p2, p3, pb);
            disect_tri_c (p2, pb, pa);
            store_outside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_12);
            pb = intersect_vert_line (qa[1], line_23);
            disect_tri_c (p2, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pb, p3, p1);
          }
        else
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_31);
            disect_tri_c (p3, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pa, p1, p2);
          }
          else
            store_outside (p1, p2, p3);
            //disect_tri_c (p1, p2, p3);
    else
      if (p1.x < qa[1].x)
        if (p2.x < qa[1].x)
          if (p3.x < qa[1].x)
//            disect_tri_c (p1, p2, p3);
            store_outside (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_31);
            store_outside (p1, p2, pb);
            store_outside (pa, pb, p2);
            disect_tri_c (pa, p3, pb);
          }
        else
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_12);
            store_outside (p1, pa, p3);
            store_outside (p1, pb, pa);
            disect_tri_c (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_31);
            pb = intersect_vert_line (qa[1], line_12);
            store_outside (p1, pb, pa);
            disect_tri_c (pa, pb, p3);
            disect_tri_c (p3, pb, p2);
          }
      else
        if (p2.x < qa[1].x)
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_12);
            pb = intersect_vert_line (qa[1], line_31);
            store_outside (p2, p3, pb);
            store_outside (p2, pb, pa);
            disect_tri_c (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_12);
            pb = intersect_vert_line (qa[1], line_23);
            store_outside (p2, pb, pa);
            disect_tri_c (pa, pb, p1);
            disect_tri_c (pb, p3, p1);
          }
        else
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_31);
            store_outside (p3, pb, pa);
            disect_tri_c (pa, pb, p1);
            disect_tri_c (pa, p1, p2);
          }
          else
            disect_tri_c (p1, p2, p3);
  else
  if (qa[1].x < qb[1].x)
    if (point_rel_line (p1, line[1]) < 0)
      if (point_rel_line (p2, line[1]) < 0)
        if (point_rel_line (p3, line[1]) < 0)
          store_outside (p1, p2, p3);
        else {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_31);
          pb = line_intersect (line[1], line_23);
          store_outside (p1, p2, pa);
          store_outside (pa, p2, pb);
          disect_tri_c (pa, pb, p3);
        }
      else
        if (point_rel_line (p3, line[1]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_23);
          pb = line_intersect (line[1], line_12);
//          printf ("here2");
          store_outside (p1, pa, p3);
          store_outside (p1, pb, pa);
          disect_tri_c (pa, pb, p2);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_31);
          pb = line_intersect (line[1], line_12);
          store_outside (p1, pb, pa);
          disect_tri_c (pa, pb, p3);
          disect_tri_c (p3, pb, p2);
        }
    else
      if (point_rel_line (p2, line[1]) < 0)
        if (point_rel_line (p3, line[1]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_12);
          pb = line_intersect (line[1], line_31);
          store_outside (p2, p3, pb);
          store_outside (p2, pb, pa);
          disect_tri_c (pa, pb, p1);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_12);
          pb = line_intersect (line[1], line_23);
          store_outside (p2, pb, pa);
          disect_tri_c (pa, pb, p1);
          disect_tri_c (pb, p3, p1);
        }
      else
        if (point_rel_line (p3, line[1]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_23);
          pb = line_intersect (line[1], line_31);
          store_outside (p3, pb, pa);
          disect_tri_c (pa, pb, p1);
          disect_tri_c (pa, p1, p2);
        }
        else
          disect_tri_c (p1, p2, p3);
  else if (qa[1].x > qb[1].x)
//***************************************************************************/
    if (point_rel_line (p1, line[1]) < 0)
      if (point_rel_line (p2, line[1]) < 0)
        if (point_rel_line (p3, line[1]) < 0)
          disect_tri_c (p1, p2, p3);
        else {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_31);
          pb = line_intersect (line[1], line_23);
          disect_tri_c (p1, p2, pa);
          disect_tri_c (pa, p2, pb);
          store_outside (pa, pb, p3);
        }
      else
        if (point_rel_line (p3, line[1]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_23);
          pb = line_intersect (line[1], line_12);
          disect_tri_c (p1, pa, p3);
          disect_tri_c (p1, pb, pa);
          store_outside (pa, pb, p2);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_31);
          pb = line_intersect (line[1], line_12);
          disect_tri_c (p1, pb, pa);
          store_outside (pa, pb, p3);
          store_outside (p3, pb, p2);
        }
    else
      if (point_rel_line (p2, line[1]) < 0)
        if (point_rel_line (p3, line[1]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_12);
          pb = line_intersect (line[1], line_31);
          disect_tri_c (p2, p3, pb);
          disect_tri_c (p2, pb, pa);
          store_outside (pa, pb, p1);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_12);
          pb = line_intersect (line[1], line_23);
          disect_tri_c (p2, pb, pa);
          store_outside (pa, pb, p1);
          store_outside (pb, p3, p1);
        }
      else
        if (point_rel_line (p3, line[1]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[1], line_23);
          pb = line_intersect (line[1], line_31);
          disect_tri_c (p3, pb, pa);
          store_outside (pa, pb, p1);
          store_outside (pa, p1, p2);
        }
        else
          store_outside (p1, p2, p3);
/***************************************************************************/
  else
    if (qa[1].y < qb[1].y)
      if (p1.x < qa[1].x)
        if (p2.x < qa[1].x)
          if (p3.x < qa[1].x)
            disect_tri_c  (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_31);
            disect_tri_c (p1, p2, pb);
            disect_tri_c (pa, pb, p2);
            store_outside (pa, p3, pb);
          }
        else
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_12);
            disect_tri_c (p1, pa, p3);
            disect_tri_c (p1, pb, pa);
            store_outside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_31);
            pb = intersect_vert_line (qa[1], line_12);
            disect_tri_c (p1, pb, pa);
            store_outside (pa, pb, p3);
            store_outside (p3, pb, p2);
          }
      else
        if (p2.x < qa[1].x)
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_12);
            pb = intersect_vert_line (qa[1], line_31);
            disect_tri_c (p2, p3, pb);
            disect_tri_c (p2, pb, pa);
            store_outside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_12);
            pb = intersect_vert_line (qa[1], line_23);
            disect_tri_c (p2, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pb, p3, p1);
          }
        else
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_31);
            disect_tri_c (p3, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pa, p1, p2);
          }
          else // *** possible bug
            store_outside (p1, p2, p3);
//            disect_tri_c (p1, p2, p3);
    else /*****************************************************************/
      if (p1.x < qa[1].x)
        if (p2.x < qa[1].x)
          if (p3.x < qa[1].x)
//            disect_tri_c (p1, p2, p3); // *** possible bug
            store_outside (p1, p2, p2);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_31);
            store_outside (p1, p2, pb);
            store_outside (pa, pb, p2);
            disect_tri_c (pa, p3, pb);
          }
        else
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_12);
            store_outside (p1, pa, p3);
            store_outside (p1, pb, pa);
            disect_tri_c (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_31);
            pb = intersect_vert_line (qa[1], line_12);
            store_outside (p1, pb, pa);
            disect_tri_c (pa, pb, p3);
            disect_tri_c (p3, pb, p2);
          }
      else
        if (p2.x < qa[1].x)
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_12);
            pb = intersect_vert_line (qa[1], line_31);
            store_outside (p2, p3, pb);
            store_outside (p2, pb, pa);
            disect_tri_c (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_12);
            pb = intersect_vert_line (qa[1], line_23);
            store_outside (p2, pb, pa);
            disect_tri_c (pa, pb, p1);
            disect_tri_c (pb, p3, p1);
          }
        else
          if (p3.x < qa[1].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[1], line_23);
            pb = intersect_vert_line (qa[1], line_31);
            store_outside (p3, pb, pa);
            disect_tri_c (pa, pb, p1);
            disect_tri_c (pa, p1, p2);
          }
          else
            disect_tri_c (p1, p2, p3);
//*/
}


inline void tri_cut::disect_tri_c (point_2d p1, point_2d p2, point_2d p3)
{
//  disect_tri_d (p1, p2, p3);
//  store_outside (p1, p2, p3, 15);
//  return;
  
  line_equat line_12, line_23, line_31;

  calc_2d_dydx_line (&line_12, p1, p2);
  calc_2d_dydx_line (&line_23, p2, p3);
  calc_2d_dydx_line (&line_31, p3, p1);

  int color1 = 4, color2 = 9;
  
//  printf ("<%f>", point_rel_line (p1, line[2]));
//  printf ("<%f>", point_rel_line (p2, line[2]));
//  printf ("<%f>", point_rel_line (p3, line[2]));
  
  if (!edge_exist[2]) {
    disect_tri_d (p1, p2, p3);
    return;
  }

  if (qa[2].x < qb[2].x)
    if (point_rel_line (p1, line[2]) < 0)
      if (point_rel_line (p2, line[2]) < 0)
        if (point_rel_line (p3, line[2]) < 0)
          store_outside (p1, p2, p3);
        else {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_31);
          pb = line_intersect (line[2], line_23);
          store_outside (p1, p2, pa);
          store_outside (pa, p2, pb);
          disect_tri_d (pa, pb, p3);
        }
      else
        if (point_rel_line (p3, line[2]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_23);
          pb = line_intersect (line[2], line_12);
//          printf ("here2");
          store_outside (p1, pa, p3);
          store_outside (p1, pb, pa);
          disect_tri_d (pa, pb, p2);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_31);
          pb = line_intersect (line[2], line_12);
          store_outside (p1, pb, pa);
          disect_tri_d (pa, pb, p3);
          disect_tri_d (p3, pb, p2);
        }
    else
      if (point_rel_line (p2, line[2]) < 0)
        if (point_rel_line (p3, line[2]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_12);
          pb = line_intersect (line[2], line_31);
          store_outside (p2, p3, pb);
          store_outside (p2, pb, pa);
          disect_tri_d (pa, pb, p1);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_12);
          pb = line_intersect (line[2], line_23);
          store_outside (p2, pb, pa);
          disect_tri_d (pa, pb, p1);
          disect_tri_d (pb, p3, p1);
        }
      else
        if (point_rel_line (p3, line[2]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_23);
          pb = line_intersect (line[2], line_31);
          store_outside (p3, pb, pa);
          disect_tri_d (pa, pb, p1);
          disect_tri_d (pa, p1, p2);
        }
        else
          disect_tri_d (p1, p2, p3);
  else if (qa[2].x > qb[2].x)
    if (point_rel_line (p1, line[2]) < 0)
      if (point_rel_line (p2, line[2]) < 0)
        if (point_rel_line (p3, line[2]) < 0)
          disect_tri_d (p1, p2, p3);
        else {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_31);
          pb = line_intersect (line[2], line_23);
          disect_tri_d (p1, p2, pa);
          disect_tri_d (pa, p2, pb);
          store_outside (pa, pb, p3);
        }
      else
        if (point_rel_line (p3, line[2]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_23);
          pb = line_intersect (line[2], line_12);
          disect_tri_d (p1, pa, p3);
          disect_tri_d (p1, pb, pa);
          store_outside (pa, pb, p2);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_31);
          pb = line_intersect (line[2], line_12);
          disect_tri_d (p1, pb, pa);
          store_outside (pa, pb, p3);
          store_outside (p3, pb, p2);
        }
    else
      if (point_rel_line (p2, line[2]) < 0)
        if (point_rel_line (p3, line[2]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_12);
          pb = line_intersect (line[2], line_31);
          disect_tri_d (p2, p3, pb);
          disect_tri_d (p2, pb, pa);
          store_outside (pa, pb, p1);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_12);
          pb = line_intersect (line[2], line_23);
          disect_tri_d (p2, pb, pa);
          store_outside (pa, pb, p1);
          store_outside (pb, p3, p1);
        }
      else
        if (point_rel_line (p3, line[2]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[2], line_23);
          pb = line_intersect (line[2], line_31);
          disect_tri_d (p3, pb, pa);
          store_outside (pa, pb, p1);
          store_outside (pa, p1, p2);
        }
        else
          store_outside (p1, p2, p3);
  else
    if (qa[2].y < qb[2].y)
      if (p1.x < qa[2].x)
        if (p2.x < qa[2].x)
          if (p3.x < qa[2].x)
//            store_outside (p1, p2, p3);
            disect_tri_d (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_23);
            pb = intersect_vert_line (qa[2], line_31);
            disect_tri_d (p1, p2, pb);
            disect_tri_d (pa, pb, p2);
            store_outside (pa, p3, pb);
          }
        else
          if (p3.x < qa[2].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_23);
            pb = intersect_vert_line (qa[2], line_12);
            disect_tri_d (p1, pa, p3);
            disect_tri_d (p1, pb, pa);
            store_outside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_31);
            pb = intersect_vert_line (qa[2], line_12);
            disect_tri_d (p1, pb, pa);
            store_outside (pa, pb, p3);
            store_outside (p3, pb, p2);
          }
      else
        if (p2.x < qa[2].x)
          if (p3.x < qa[2].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_12);
            pb = intersect_vert_line (qa[2], line_31);
            disect_tri_d (p2, p3, pb);
            disect_tri_d (p2, pb, pa);
            store_outside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_12);
            pb = intersect_vert_line (qa[2], line_23);
            disect_tri_d (p2, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pb, p3, p1);
          }
        else
          if (p3.x < qa[2].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_23);
            pb = intersect_vert_line (qa[2], line_31);
            disect_tri_d (p3, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pa, p1, p2);
          }
          else
//            disect_tri_d (p1, p2, p3);
            store_outside (p1, p2, p3);
    else
      if (p1.x < qa[2].x)
        if (p2.x < qa[2].x)
          if (p3.x < qa[2].x)
            store_outside (p1, p2, p3);
//            disect_tri_d (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_23);
            pb = intersect_vert_line (qa[2], line_31);
            store_outside (p1, p2, pb);
            store_outside (pa, pb, p2);
            disect_tri_d (pa, p3, pb);
          }
        else
          if (p3.x < qa[2].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_23);
            pb = intersect_vert_line (qa[2], line_12);
            store_outside (p1, pa, p3);
            store_outside (p1, pb, pa);
            disect_tri_d (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_31);
            pb = intersect_vert_line (qa[2], line_12);
            store_outside (p1, pb, pa);
            disect_tri_d (pa, pb, p3);
            disect_tri_d (p3, pb, p2);
          }
      else
        if (p2.x < qa[2].x)
          if (p3.x < qa[2].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_12);
            pb = intersect_vert_line (qa[2], line_31);
            store_outside (p2, p3, pb);
            store_outside (p2, pb, pa);
            disect_tri_d (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_12);
            pb = intersect_vert_line (qa[2], line_23);
            store_outside (p2, pb, pa);
            disect_tri_d (pa, pb, p1);
            disect_tri_d (pb, p3, p1);
          }
        else
          if (p3.x < qa[2].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[2], line_23);
            pb = intersect_vert_line (qa[2], line_31);
            store_outside (p3, pb, pa);
            disect_tri_d (pa, pb, p1);
            disect_tri_d (pa, p1, p2);
          }
          else
            disect_tri_d (p1, p2, p3);
//*/
}


inline void tri_cut::disect_tri_d (point_2d p1, point_2d p2, point_2d p3)
{
//  draw_solid_tri (p1, p2, p3, 15);
//  return;
  
  line_equat line_12, line_23, line_31;

  calc_2d_dydx_line (&line_12, p1, p2);
  calc_2d_dydx_line (&line_23, p2, p3);
  calc_2d_dydx_line (&line_31, p3, p1);

  int color1 = 4, color2 = 9;

  if (!edge_exist[3]) {
    store_inside (p1, p2, p3);
    return;
  }
  
  if (approx_equal(qa[3].x, qb[3].x))
    if (qa[3].y < qb[3].y)
      if (p1.x < qa[3].x)
        if (p2.x < qa[3].x)
          if (p3.x < qa[3].x)
            store_inside (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_31);
            store_inside (p1, p2, pb);
            store_inside (pa, pb, p2);
            store_outside (pa, p3, pb);
          }
        else
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_12);
            store_inside (p1, pa, p3);
            store_inside (p1, pb, pa);
            store_outside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_31);
            pb = intersect_vert_line (qa[3], line_12);
            store_inside (p1, pb, pa);
            store_outside (pa, pb, p3);
            store_outside (p3, pb, p2);
          }
      else
        if (p2.x < qa[3].x)
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_12);
            pb = intersect_vert_line (qa[3], line_31);
            store_inside (p2, p3, pb);
            store_inside (p2, pb, pa);
            store_outside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_12);
            pb = intersect_vert_line (qa[3], line_23);
            store_inside (p2, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pb, p3, p1);
          }
        else
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_31);
            store_inside (p3, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pa, p1, p2);
          }
          else
//            store_inside (p1, p2, p3);
            store_outside (p1, p2, p3);
    else
      if (p1.x < qa[3].x)
        if (p2.x < qa[3].x)
          if (p3.x < qa[3].x)
            store_outside (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_31);
            store_outside (p1, p2, pb);
            store_outside (pa, pb, p2);
            store_inside (pa, p3, pb);
          }
        else
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_12);
            store_outside (p1, pa, p3);
            store_outside (p1, pb, pa);
            store_inside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_31);
            pb = intersect_vert_line (qa[3], line_12);
            store_outside (p1, pb, pa);
            store_inside (pa, pb, p3);
            store_inside (p3, pb, p2);
          }
      else
        if (p2.x < qa[3].x)
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_12);
            pb = intersect_vert_line (qa[3], line_31);
            store_outside (p2, p3, pb);
            store_outside (p2, pb, pa);
            store_inside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_12);
            pb = intersect_vert_line (qa[3], line_23);
            store_outside (p2, pb, pa);
            store_inside (pa, pb, p1);
            store_inside (pb, p3, p1);
          }
        else
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_31);
            store_outside (p3, pb, pa);
            store_inside (pa, pb, p1);
            store_inside (pa, p1, p2);
          }
          else
            store_inside (p1, p2, p3);
  else
  
  if (qa[3].x < qb[3].x)
    if (point_rel_line (p1, line[3]) < 0)
      if (point_rel_line (p2, line[3]) < 0)
        if (point_rel_line (p3, line[3]) < 0)
          store_outside (p1, p2, p3);
        else {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_31);
          pb = line_intersect (line[3], line_23);
          store_outside (p1, p2, pa);
          store_outside (pa, p2, pb);
          store_inside (pa, pb, p3);
        }
      else
        if (point_rel_line (p3, line[3]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_23);
          pb = line_intersect (line[3], line_12);
//          printf ("here2");
          store_outside (p1, pa, p3);
          store_outside (p1, pb, pa);
          store_inside (pa, pb, p2);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_31);
          pb = line_intersect (line[3], line_12);
          store_outside (p1, pb, pa);
          store_inside (pa, pb, p3);
          store_inside (p3, pb, p2);
        }
    else
      if (point_rel_line (p2, line[3]) < 0)
        if (point_rel_line (p3, line[3]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_12);
          pb = line_intersect (line[3], line_31);
          store_outside (p2, p3, pb);
          store_outside (p2, pb, pa);
          store_inside (pa, pb, p1);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_12);
          pb = line_intersect (line[3], line_23);
          store_outside (p2, pb, pa);
          store_inside (pa, pb, p1);
          store_inside (pb, p3, p1);
        }
      else
        if (point_rel_line (p3, line[3]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_23);
          pb = line_intersect (line[3], line_31);
          store_outside (p3, pb, pa);
          store_inside (pa, pb, p1);
          store_inside (pa, p1, p2);
        }
        else
          store_inside (p1, p2, p3);
  else if (qa[3].x > qb[3].x)
    if (point_rel_line (p1, line[3]) < 0)
      if (point_rel_line (p2, line[3]) < 0)
        if (point_rel_line (p3, line[3]) < 0)
          store_inside (p1, p2, p3);
        else {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_31);
          pb = line_intersect (line[3], line_23);
          store_inside (p1, p2, pa);
          store_inside (pa, p2, pb);
          store_outside (pa, pb, p3);
        }
      else
        if (point_rel_line (p3, line[3]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_23);
          pb = line_intersect (line[3], line_12);
          store_inside (p1, pa, p3);
          store_inside (p1, pb, pa);
          store_outside (pa, pb, p2);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_31);
          pb = line_intersect (line[3], line_12);
          store_inside (p1, pb, pa);
          store_outside (pa, pb, p3);
          store_outside (p3, pb, p2);
        }
    else
      if (point_rel_line (p2, line[3]) < 0)
        if (point_rel_line (p3, line[3]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_12);
          pb = line_intersect (line[3], line_31);
          store_inside (p2, p3, pb);
          store_inside (p2, pb, pa);
          store_outside (pa, pb, p1);
        }
        else {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_12);
          pb = line_intersect (line[3], line_23);
          store_inside (p2, pb, pa);
          store_outside (pa, pb, p1);
          store_outside (pb, p3, p1);
        }
      else
        if (point_rel_line (p3, line[3]) < 0) {
          point_2d pa, pb;
          pa = line_intersect (line[3], line_23);
          pb = line_intersect (line[3], line_31);
          store_inside (p3, pb, pa);
          store_outside (pa, pb, p1);
          store_outside (pa, p1, p2);
        }
        else
          store_outside (p1, p2, p3);
  else
    if (qa[3].y < qb[3].y)
      if (p1.x < qa[3].x)
        if (p2.x < qa[3].x)
          if (p3.x < qa[3].x)
//            store_outside (p1, p2, p3);
            store_inside (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_31);
            store_inside (p1, p2, pb);
            store_inside (pa, pb, p2);
            store_outside (pa, p3, pb);
          }
        else
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_12);
            store_inside (p1, pa, p3);
            store_inside (p1, pb, pa);
            store_outside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_31);
            pb = intersect_vert_line (qa[3], line_12);
            store_inside (p1, pb, pa);
            store_outside (pa, pb, p3);
            store_outside (p3, pb, p2);
          }
      else
        if (p2.x < qa[3].x)
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_12);
            pb = intersect_vert_line (qa[3], line_31);
            store_inside (p2, p3, pb);
            store_inside (p2, pb, pa);
            store_outside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_12);
            pb = intersect_vert_line (qa[3], line_23);
            store_inside (p2, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pb, p3, p1);
          }
        else
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_31);
            store_inside (p3, pb, pa);
            store_outside (pa, pb, p1);
            store_outside (pa, p1, p2);
          }
          else
//            store_inside (p1, p2, p3);
            store_outside (p1, p2, p3);
    else
      if (p1.x < qa[3].x)
        if (p2.x < qa[3].x)
          if (p3.x < qa[3].x)
//            store_inside (p1, p2, p3);
            store_outside (p1, p2, p3);
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_31);
            store_outside (p1, p2, pb);
            store_outside (pa, pb, p2);
            store_inside (pa, p3, pb);
          }
        else
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_12);
            store_outside (p1, pa, p3);
            store_outside (p1, pb, pa);
            store_inside (pa, pb, p2);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_31);
            pb = intersect_vert_line (qa[3], line_12);
            store_outside (p1, pb, pa);
            store_inside (pa, pb, p3);
            store_inside (p3, pb, p2);
          }
      else
        if (p2.x < qa[3].x)
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_12);
            pb = intersect_vert_line (qa[3], line_31);
            store_outside (p2, p3, pb);
            store_outside (p2, pb, pa);
            store_inside (pa, pb, p1);
          }
          else {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_12);
            pb = intersect_vert_line (qa[3], line_23);
            store_outside (p2, pb, pa);
            store_inside (pa, pb, p1);
            store_inside (pb, p3, p1);
          }
        else
          if (p3.x < qa[3].x) {
            point_2d pa, pb;
            pa = intersect_vert_line (qa[3], line_23);
            pb = intersect_vert_line (qa[3], line_31);
            store_outside (p3, pb, pa);
            store_inside (pa, pb, p1);
            store_inside (pa, p1, p2);
          }
          else
            store_inside (p1, p2, p3);
//*/
}


inline void tri_cut::store_inside (point_2d p1, point_2d p2, point_2d p3)
{
  inside->p1 = p1;
  inside->p2 = p2;
  inside->p3 = p3;
  inside++;
}


inline void tri_cut::store_outside (point_2d p1, point_2d p2, point_2d p3)
{
  outside->p1 = p1;
  outside->p2 = p2;
  outside->p3 = p3;
  outside++;
}


inline point_2d line_intersect (line_equat line1, line_equat line2)
{
  point_2d p;

  if (line2.m == _inf_double) {
    p.x = line2.b;
    p.y = line1.m * p.x + line1.b;
//    getch();
  }
  else if (line2.m == -_inf_double) {
//    exit(321);
    p.x = line2.b;
    p.y = line1.m * p.x + line1.b;
//    getch();
  }
  else {
    p.x = (line2.b - line1.b) / (line1.m - line2.m);
    p.y = line1.m * p.x + line1.b;
  }
  
  return p;
}


void draw_line (line_equat line, int color)
{
  if (line.m == _inf_double) {
    for (int y = 0; y < 199; y++)
      pxl ((int)line.b, y, color);
  }
  else if (fabs(line.m) < 1) {
    double y = line.b;
    for (int x = 0; x < 320; x++, y += line.m)
      pxl (x, int(y), color);
  }
}


void draw_line (point_2d p1, point_2d p2, int color)
{
  line_equat line;
  
  calc_2d_dydx_line (&line, p1, p2);
  
  if (p1.x == p2.x)
    if (p1.y < p2.y)
      for (int y = (int)p1.y; y <= (int)p2.y; y++)
        pxl ((int)line.b, y, color);
    else
      for (int y = (int)p2.y; y <= (int)p1.y; y++)
        pxl ((int)line.b, y, color);
  else if (fabs(line.m) < 1)
    if (p1.x < p2.x) {
      double y = p1.y;
      for (int x = (int)p1.x; x <= (int)p2.x; x++, y += line.m)
        pxl (x, int(y), color);
    }
    else {
      double y = p2.y;
      for (int x = (int)p2.x; x <= (int)p1.x; x++, y += line.m)
        pxl (x, int(y), color);
    }
  else {
    calc_2d_dxdy_line (&line, p1, p2);
    if (p1.y < p2.y) {
      double x = p1.x;
      for (int y = (int)p1.y; y <= (int)p2.y; y++, x += line.m)
        pxl (int(x), y, color);
    }
    else {
      double x = p2.x;
      for (int y = (int)p2.y; y <= (int)p1.y; y++, x += line.m)
        pxl (int(x), y, color);
    }
  }
}


inline point_2d intersect_vert_line (point_2d point, line_equat line)
{
  point_2d p;

  p.x = point.x;
  p.y = line.m * point.x + line.b;
  
  return p;
}


inline void group_rot (rot_data d, double* x, double* y)
{
  double bx = *x * d.x_sub;
  double by = *y * d.y_sub;
  *x = (by - bx) * d.inv_dm;
  *y = d.m * *x + bx;
}


inline void group_rot
  (rot_data d, double in_x, double in_y, double* out_x, double* out_y)
{
  double bx = in_x * d.x_sub;
  double by = in_y * d.y_sub;
  *out_x = (by - bx) * d.inv_dm;
  *out_y = d.m * in_x + bx;
}


inline double simplify_angle (double a)
{
  // returns the equivalent angle between 0 and 2pi
  
  return a - (PI * 2 * floor(a / (PI * 2)));
}


inline void simplify_angle (double* a)
{
  // converts the angle to the equivalent angle between 0 and 2pi
  
//  *a -= (PI * 2 * floor(*a / (PI * 2)));
}


inline void calc_rot_data (rot_data* dat, double a)
{
  dat->m = -1 / tan(a);
  dat->inv_dm = 1 / (dat->m - tan(a));
  dat->x_sub = sin(a) - dat->m * cos(a);
  dat->y_sub = sin(a + PI * .5) - tan(a) * cos(a + PI * .5);
}


void calc_rel_3d_points (view_type view, point_type* p, point_type* last)
{
  for (point_type* i = p; i <= last; i++) {
    i->rel.x = i->abs.x - view.camera.pos.x;
    i->rel.y = i->abs.y - view.camera.pos.y;
    i->rel.z = i->abs.z - view.camera.pos.z;
  }

  if (!approx_equal(view.camera.angle_xz, 0))
    if (approx_equal(view.camera.angle_xz, 1 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.x;
        i->rel.x = -i->rel.z;
        i->rel.z = temp;
      }
    else if (approx_equal(view.camera.angle_xz, 2 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        i->rel.x = -i->rel.x;
        i->rel.z = -i->rel.z;
      }
    else if (approx_equal(view.camera.angle_xz, 3 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.x;
        i->rel.x = i->rel.z;
        i->rel.z = -temp;
      }
    else {
      calc_rot_data (&view.rot_xz, view.camera.angle_xz);
      for (point_type* i = p; i <= last; i++)
        group_rot (view.rot_xz, &i->rel.x, &i->rel.z);
    }

  if (!approx_equal(view.camera.angle_yz, 0))
    if (approx_equal(view.camera.angle_yz, 1 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.z;
        i->rel.z = -i->rel.y;
        i->rel.y = temp;
      }
    else if (approx_equal(view.camera.angle_yz, 2 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        i->rel.z = -i->rel.z;
        i->rel.y = -i->rel.y;
      }
    else if (approx_equal(view.camera.angle_yz, 3 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.z;
        i->rel.z = i->rel.y;
        i->rel.y = -temp;
      }
    else {
      calc_rot_data (&view.rot_yz, view.camera.angle_yz);
      for (point_type* i = p; i <= last; i++)
        group_rot (view.rot_yz, &i->rel.z, &i->rel.y);
    }

  if (!approx_equal(view.camera.angle_xy, 0))
    if (approx_equal(view.camera.angle_xy, 1 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.x;
        i->rel.x = -i->rel.y;
        i->rel.y = temp;
      }
    else if (approx_equal(view.camera.angle_xy, 2 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        i->rel.x = -i->rel.x;
        i->rel.y = -i->rel.y;
      }
    else if (approx_equal(view.camera.angle_xy, 3 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.x;
        i->rel.x = i->rel.y;
        i->rel.y = -temp;
      }
    else {
      calc_rot_data (&view.rot_xy, view.camera.angle_xy);
      for (point_type* i = p; i <= last; i++)
        group_rot (view.rot_xy, &i->rel.x, &i->rel.y);
    }
}


void init_view (view_type* view, double field_x, double field_y)
{
  view->camera.pos.x = 10;
  view->camera.pos.y = -100;
  view->camera.pos.z = 75;
  view->camera.angle_xz = 0;//;
  view->camera.angle_yz = 0;
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


point_2d map_to_scrn (view_type view, point_type p)
{
  point_2d p2d;

  double inv_z = 1 / p.rel.z;
  p2d.x = view.center.x + view.zoom.x * p.rel.x * inv_z;
  p2d.y = view.center.y + view.zoom.y * p.rel.y * inv_z;
  
  return p2d;
}


point_2d map_to_scrn (view_type view, point_3d p)
{
  point_2d p2d;

  double inv_z = 1 / p.z;
  p2d.x = view.center.x + view.zoom.x * p.x * inv_z;
  p2d.y = view.center.y + view.zoom.y * p.y * inv_z;
  
  return p2d;
}


void update_player (view_type* view)
{
  double rec_inc = 2;

  int temp = keyb();

  if (temp == 2)
    _play_mode = !_play_mode;

  if (!_play_mode) {
    if (temp == 1) {
      set_vesa_mode(0x03);
      exit(0);
    }
    else if (temp == 3) {
      view->camera.angle_yz += rec_inc * .025;
      if (view->camera.angle_yz >= 2 * PI)
        view->camera.angle_yz = 0;
//      simplify_angle (&view->camera.angle_yz);
    }
    else if (temp == 4) {
      view->camera.angle_yz -= rec_inc * .025;
//      simplify_angle (&view->camera.angle_yz);
      if (view->camera.angle_yz < 0)
        view->camera.angle_yz += 2 * PI;
    }
    else if (temp == 72) {
      view->camera.pos.x += sin(view->camera.angle_xz) * cos(view->camera.angle_yz) * rec_inc;
      view->camera.pos.y -= sin(view->camera.angle_yz) * rec_inc;
      view->camera.pos.z += cos(view->camera.angle_xz) * cos(view->camera.angle_yz) * rec_inc;
    }
    else if (temp == 80) {
      view->camera.pos.x -= sin(view->camera.angle_xz) * cos(view->camera.angle_yz) * rec_inc;
      view->camera.pos.y += sin(view->camera.angle_yz) * rec_inc;
      view->camera.pos.z -= cos(view->camera.angle_xz) * cos(view->camera.angle_yz) * rec_inc;
    }
    else if (temp == 16) {
      view->camera.pos.x -= sin(view->camera.angle_xz + PI / 2) * rec_inc;
      view->camera.pos.z -= cos(view->camera.angle_xz + PI / 2) * rec_inc;
    }
    else if (temp == 18) {
      view->camera.pos.x += sin(view->camera.angle_xz + PI / 2) * rec_inc;
      view->camera.pos.z += cos(view->camera.angle_xz + PI / 2) * rec_inc;
    }
    else if (temp == 'K') {
      view->camera.angle_xz -= rec_inc * PI / 100;
      if (view->camera.angle_xz < 0)
        view->camera.angle_xz += 2 * PI;
//      simplify_angle (&view->camera.angle_xz);
    }
    else if (temp == 'M') {
      view->camera.angle_xz += rec_inc * PI / 100;
      if (view->camera.angle_xz >= 2 * PI)
        view->camera.angle_xz = 0;
//      simplify_angle (&view->camera.angle_xz);
    }
    else if (temp == 30) {
      view->camera.angle_xy += rec_inc * PI / 100;
      if (view->camera.angle_xy >= 2 * PI)
        view->camera.angle_xy = 0;
//      simplify_angle (&view->camera.angle_xy);
    }
    else if (temp == 32) {
      view->camera.angle_xy -= rec_inc * PI / 100;
      if (view->camera.angle_xy < 0)
        view->camera.angle_xy += 2 * PI;
//      simplify_angle (&view->camera.angle_xy);
    }
    else if (temp == 2)
      ;
  }
  else {
    if (temp == 1) {
      set_vesa_mode(0x03);
      exit(0);
    }
    else if (temp == 2) {
      ;//    _debug[0] = -_debug[0];
    }
    else if (temp == 3) {
      view->camera.angle_yz += rec_inc * .025;
      simplify_angle (&view->camera.angle_yz);
    }
    else if (temp == 4) {
      view->camera.angle_yz -= rec_inc * .025;
      simplify_angle (&view->camera.angle_yz);
    }
    else if (temp == 72) {
      view->camera.pos.x += sin(view->camera.angle_xz) * rec_inc;
      view->camera.pos.z += cos(view->camera.angle_xz) * rec_inc;
    }
    else if (temp == 80) {
      view->camera.pos.x -= sin(view->camera.angle_xz) * rec_inc;
      view->camera.pos.z -= cos(view->camera.angle_xz) * rec_inc;
    }
    else if (temp == 16) {
      view->camera.pos.x -= sin(view->camera.angle_xz + PI / 2) * rec_inc;
      view->camera.pos.z -= cos(view->camera.angle_xz + PI / 2) * rec_inc;
    }
    else if (temp == 18) {
      view->camera.pos.x += sin(view->camera.angle_xz + PI / 2) * rec_inc;
      view->camera.pos.z += cos(view->camera.angle_xz + PI / 2) * rec_inc;
    }
    else if (temp == 'K') {
      view->camera.angle_xz -= rec_inc * PI / 100;
      simplify_angle (&view->camera.angle_xz);
    }
    else if (temp == 'M') {
      view->camera.angle_xz += rec_inc * PI / 100;
      simplify_angle (&view->camera.angle_xz);
    }
    else if (temp == 30) {
      view->camera.angle_xy += rec_inc * PI / 100;
      simplify_angle (&view->camera.angle_xy);
    }
    else if (temp == 32) {
      view->camera.angle_xy -= rec_inc * PI / 100;
      simplify_angle (&view->camera.angle_xy);
    }
    else if (temp == 2)
      ;
  }
}


int keyb()
{
 union REGS r;
 
 r.x.ax = 0x0C06;
 r.x.dx = 0x00FF;
 int86(0x21, &r, &r);

 return inp(0x60);
}


void clear_screen()
{
  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++)
      pxl (x, y, 0, 0, 0);
}


inline void draw_tri_3d (tri_3d tri, poly_dat dat, view_type* view)
{
  if (tri.p1->rel.z >= view->z_cutoff)
    if (tri.p2->rel.z >= view->z_cutoff)
      if (tri.p3->rel.z >= view->z_cutoff)
        edge_clip_left (tri.p1->rel, tri.p2->rel, tri.p3->rel, dat, view);
      else {
        point_3d a = intrapolate_3d (tri.p1->rel, tri.p3->rel, view->z_cutoff);
        point_3d b = intrapolate_3d (tri.p2->rel, tri.p3->rel, view->z_cutoff);
        edge_clip_left (tri.p1->rel, b, a, dat, view);
        edge_clip_left (tri.p1->rel, tri.p2->rel, b, dat, view);
      }
    else
      if (tri.p3->rel.z >= view->z_cutoff) {
        point_3d a = intrapolate_3d (tri.p1->rel, tri.p2->rel, view->z_cutoff);
        point_3d b = intrapolate_3d (tri.p2->rel, tri.p3->rel, view->z_cutoff);
        edge_clip_left (tri.p1->rel, a, tri.p3->rel, dat, view);
        edge_clip_left (a, b, tri.p3->rel, dat, view);
      }
      else {
        point_3d a = intrapolate_3d (tri.p1->rel, tri.p3->rel, view->z_cutoff);
        point_3d b = intrapolate_3d (tri.p1->rel, tri.p2->rel, view->z_cutoff);
        edge_clip_left (tri.p1->rel, b, a, dat, view);
      }
  else
    if (tri.p2->rel.z >= view->z_cutoff)
      if (tri.p3->rel.z >= view->z_cutoff) {
        point_3d a = intrapolate_3d (tri.p1->rel, tri.p2->rel, view->z_cutoff);
        point_3d b = intrapolate_3d (tri.p1->rel, tri.p3->rel, view->z_cutoff);
        edge_clip_left (a, tri.p2->rel, tri.p3->rel, dat, view);
        edge_clip_left (b, a, tri.p3->rel, dat, view);
      }
      else {
        point_3d a = intrapolate_3d (tri.p1->rel, tri.p2->rel, view->z_cutoff);
        point_3d b = intrapolate_3d (tri.p2->rel, tri.p3->rel, view->z_cutoff);
        edge_clip_left (tri.p2->rel, b, a, dat, view);
      }
    else
      if (tri.p3->rel.z >= view->z_cutoff) {
        point_3d a = intrapolate_3d (tri.p2->rel, tri.p3->rel, view->z_cutoff);
        point_3d b = intrapolate_3d (tri.p1->rel, tri.p3->rel, view->z_cutoff);
        edge_clip_left (a, tri.p3->rel, b, dat, view);
      }
}


inline void edge_clip_left
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view)
{
//  edge_clip_right (p1, p2, p3, dat, view);

//  return;
  
  if (p1.x >= view->vis_lm * p1.z)
    if (p2.x >= view->vis_lm * p2.z)
      if (p3.x >= view->vis_lm * p3.z)
        edge_clip_right (p1, p2, p3, dat, view);
      else {
        point_3d a = clip_edge_lr (p1, p3, view->vis_lm);
        point_3d b = clip_edge_lr (p2, p3, view->vis_lm);
        edge_clip_right (p1, b, a, dat, view);
        edge_clip_right (p1, p2, b, dat, view);
      }
    else
      if (p3.x >= view->vis_lm * p3.z) {
        point_3d a = clip_edge_lr (p1, p2, view->vis_lm);
        point_3d b = clip_edge_lr (p2, p3, view->vis_lm);
        edge_clip_right (p1, a, p3, dat, view);
        edge_clip_right (a, b, p3, dat, view);
      }
      else {
        point_3d a = clip_edge_lr (p1, p3, view->vis_lm);
        point_3d b = clip_edge_lr (p1, p2, view->vis_lm);
        edge_clip_right (p1, b, a, dat, view);
      }
  else
    if (p2.x >= view->vis_lm * p2.z)
      if (p3.x >= view->vis_lm * p3.z) {
        point_3d a = clip_edge_lr (p1, p2, view->vis_lm);
        point_3d b = clip_edge_lr (p1, p3, view->vis_lm);
        edge_clip_right (a, p2, p3, dat, view);
        edge_clip_right (b, a, p3, dat, view);
      }
      else {
        point_3d a = clip_edge_lr (p1, p2, view->vis_lm);
        point_3d b = clip_edge_lr (p2, p3, view->vis_lm);
        edge_clip_right (p2, b, a, dat, view);
      }
    else
      if (p3.x >= view->vis_lm * p3.z) {
        point_3d a = clip_edge_lr (p2, p3, view->vis_lm);
        point_3d b = clip_edge_lr (p1, p3, view->vis_lm);
        edge_clip_right (a, p3, b, dat, view);
      }
}


inline void edge_clip_right
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view)
{
//  edge_clip_top (p1, p2, p3, dat, view);

//  return;
  
  if (p1.x <= view->vis_rm * p1.z)
    if (p2.x <= view->vis_rm * p2.z)
      if (p3.x <= view->vis_rm * p3.z)
        edge_clip_top (p1, p2, p3, dat, view);
      else {
        point_3d a = clip_edge_lr (p1, p3, view->vis_rm);
        point_3d b = clip_edge_lr (p2, p3, view->vis_rm);
        edge_clip_top (p1, b, a, dat, view);
        edge_clip_top (p1, p2, b, dat, view);
      }
    else
      if (p3.x <= view->vis_rm * p3.z) {
        point_3d a = clip_edge_lr (p1, p2, view->vis_rm);
        point_3d b = clip_edge_lr (p2, p3, view->vis_rm);
        edge_clip_top (p1, a, p3, dat, view);
        edge_clip_top (a, b, p3, dat, view);
      }
      else {
        point_3d a = clip_edge_lr (p1, p3, view->vis_rm);
        point_3d b = clip_edge_lr (p1, p2, view->vis_rm);
        edge_clip_top (p1, b, a, dat, view);
      }
  else
    if (p2.x <= view->vis_rm * p2.z)
      if (p3.x <= view->vis_rm * p3.z) {
        point_3d a = clip_edge_lr (p1, p2, view->vis_rm);
        point_3d b = clip_edge_lr (p1, p3, view->vis_rm);
        edge_clip_top (a, p2, p3, dat, view);
        edge_clip_top (b, a, p3, dat, view);
      }
      else {
        point_3d a = clip_edge_lr (p1, p2, view->vis_rm);
        point_3d b = clip_edge_lr (p2, p3, view->vis_rm);
        edge_clip_top (p2, b, a, dat, view);
      }
    else
      if (p3.x <= view->vis_rm * p3.z) {
        point_3d a = clip_edge_lr (p2, p3, view->vis_rm);
        point_3d b = clip_edge_lr (p1, p3, view->vis_rm);
        edge_clip_top (a, p3, b, dat, view);
      }
}


inline void edge_clip_top
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view)
{
  if (p1.y >= view->vis_tm * p1.z)
    if (p2.y >= view->vis_tm * p2.z)
      if (p3.y >= view->vis_tm * p3.z)
        edge_clip_bot (p1, p2, p3, dat, view);
      else {
        point_3d a = clip_edge_tb (p1, p3, view->vis_tm);
        point_3d b = clip_edge_tb (p2, p3, view->vis_tm);
        edge_clip_bot (p1, b, a, dat, view);
        edge_clip_bot (p1, p2, b, dat, view);
      }
    else
      if (p3.y >= view->vis_tm * p3.z) {
        point_3d a = clip_edge_tb (p1, p2, view->vis_tm);
        point_3d b = clip_edge_tb (p2, p3, view->vis_tm);
        edge_clip_bot (p1, a, p3, dat, view);
        edge_clip_bot (a, b, p3, dat, view);
      }
      else {
        point_3d a = clip_edge_tb (p1, p3, view->vis_tm);
        point_3d b = clip_edge_tb (p1, p2, view->vis_tm);
        edge_clip_bot (p1, b, a, dat, view);
      }
  else
    if (p2.y >= view->vis_tm * p2.z)
      if (p3.y >= view->vis_tm * p3.z) {
        point_3d a = clip_edge_tb (p1, p2, view->vis_tm);
        point_3d b = clip_edge_tb (p1, p3, view->vis_tm);
        edge_clip_bot (a, p2, p3, dat, view);
        edge_clip_bot (b, a, p3, dat, view);
      }
      else {
        point_3d a = clip_edge_tb (p1, p2, view->vis_tm);
        point_3d b = clip_edge_tb (p2, p3, view->vis_tm);
        edge_clip_bot (p2, b, a, dat, view);
      }
    else
      if (p3.y >= view->vis_tm * p3.z) {
        point_3d a = clip_edge_tb (p2, p3, view->vis_tm);
        point_3d b = clip_edge_tb (p1, p3, view->vis_tm);
        edge_clip_bot (a, p3, b, dat, view);
      }
}


inline void edge_clip_bot
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view)
{
  if (p1.y <= view->vis_bm * p1.z)
    if (p2.y <= view->vis_bm * p2.z)
      if (p3.y <= view->vis_bm * p3.z)
        proc_dyn_light (p1, p2, p3, dat, view);
      else {
        point_3d a = clip_edge_tb (p1, p3, view->vis_bm);
        point_3d b = clip_edge_tb (p2, p3, view->vis_bm);
        proc_dyn_light (p1, b, a, dat, view);
        proc_dyn_light (p1, p2, b, dat, view);
      }
    else
      if (p3.y <= view->vis_bm * p3.z) {
        point_3d a = clip_edge_tb (p1, p2, view->vis_bm);
        point_3d b = clip_edge_tb (p2, p3, view->vis_bm);
        proc_dyn_light (p1, a, p3, dat, view);
        proc_dyn_light (a, b, p3, dat, view);
      }
      else {
        point_3d a = clip_edge_tb (p1, p3, view->vis_bm);
        point_3d b = clip_edge_tb (p1, p2, view->vis_bm);
        proc_dyn_light (p1, b, a, dat, view);
      }
  else
    if (p2.y <= view->vis_bm * p2.z)
      if (p3.y <= view->vis_bm * p3.z) {
        point_3d a = clip_edge_tb (p1, p2, view->vis_bm);
        point_3d b = clip_edge_tb (p1, p3, view->vis_bm);
        proc_dyn_light (a, p2, p3, dat, view);
        proc_dyn_light (b, a, p3, dat, view);
      }
      else {
        point_3d a = clip_edge_tb (p1, p2, view->vis_bm);
        point_3d b = clip_edge_tb (p2, p3, view->vis_bm);
        proc_dyn_light (p2, b, a, dat, view);
      }
    else
      if (p3.y <= view->vis_bm * p3.z) {
        point_3d a = clip_edge_tb (p2, p3, view->vis_bm);
        point_3d b = clip_edge_tb (p1, p3, view->vis_bm);
        proc_dyn_light (a, p3, b, dat, view);
      }
}


inline void proc_dyn_light
  (point_3d p1, point_3d p2, point_3d p3, poly_dat dat, view_type* view)
{
  int rot;

  init_tex_plane (dat.ref[0]->rel, dat.ref[1]->rel, dat.ref[3]->rel,
    &dat, &rot, &dat.bmp, *view);
  plane_type plane;

  plane_type pl = _pl;
  tri_cut tc = _tc;
  
  tri_type ins[100], outs[100];

  int sm = 0;
  int c = 255;
  
//      tc.disect_tri (map_to_scrn (*view, p1),
//                     map_to_scrn (*view, p2),
//                     map_to_scrn (*view, p3), ins, outs);

  if (0 && (pl.b > 0) && (fabs(pl.m1) < 1000000) && (is_real_num(pl.m1)) && !pl.y_plane)
    if (dat.rot) {
      _seg_mode = 1;
      c = 0;
      for (tri_type* i = ins; i < tc.inside; i++, c+=10)
        draw_horz_tex_tri (i->p1, i->p2, i->p3, dat, c, *view);
      _seg_mode = 0;
      c = 128;
      for (tri_type* i = outs; i < tc.outside; i++, c+=10)
        draw_horz_tex_tri (i->p1, i->p2, i->p3, dat, c, *view);
    }
    else {
      _seg_mode = 1;
      c = 0;
      for (tri_type* i = ins; i < tc.inside; i++, c+=10)
        draw_vert_tex_tri (i->p1, i->p2, i->p3, dat, c, *view);
      _seg_mode = 0;
      c = 128;
      for (tri_type* i = outs; i < tc.outside; i++, c+=10)
        draw_vert_tex_tri (i->p1, i->p2, i->p3, dat, c, *view);
    }
  else
    if (dat.rot) {
      _seg_mode = 0;
      init_horz_tri (map_to_scrn (*view, p1),
                     map_to_scrn (*view, p2),
                     map_to_scrn (*view, p3), &_uv_init, dat.scan_m);
      draw_horz_tex_tri (map_to_scrn (*view, p1),
                         map_to_scrn (*view, p2),
                         map_to_scrn (*view, p3), dat, c, *view);
    }
    else {
      _seg_mode = 0;
      init_vert_tri (map_to_scrn (*view, p1),
                     map_to_scrn (*view, p2),
                     map_to_scrn (*view, p3), &_uv_init, dat.scan_m);
      draw_vert_tex_tri (map_to_scrn (*view, p1),
                         map_to_scrn (*view, p2),
                         map_to_scrn (*view, p3), dat, c, *view);
    }
//  pxl (tc.qa[0], 255, 255, 255);
//  pxl (tc.qa[1], 255, 255, 255);
//  pxl (tc.qa[2], 255, 255, 255);
//  pxl (tc.qa[3], 255, 255, 255);
}


inline point_3d calc_light_corner
  (plane_type plane, double x_len, double y_len)
{
  point_3d p;
  double a;

  a = atan(plane.m1);
  p.z = plane.b + x_len * sin(a);
  p.x = x_len * cos(a);

  a = atan(plane.m2);
  p.z += y_len * sin(a);
  p.y = y_len * cos(a);

  return p;
}


inline double deg_to_rad (double deg)
{
  return deg * PI / 180;
}



inline bool is_approx_zero (double n)
{
  if (fabs(n) < .000001)
    return true;
  else
    return false;
}


inline point_3d intrapolate_3d (point_3d p1, point_3d p2, double z)
{
  point_3d temp;
  double dx = (p2.x - p1.x) / (p2.z - p1.z);
  double dy = (p2.y - p1.y) / (p2.z - p1.z);

  temp.x = p1.x + dx * (z - p1.z);
  temp.y = p1.y + dy * (z - p1.z);
  temp.z = z;

  return temp;
}


inline point_3d intrapolate_3d (point_type p1, point_type p2, double z)
{
  point_3d temp;
  double dx = (p2.rel.x - p1.rel.x) / (p2.rel.z - p1.rel.z);
  double dy = (p2.rel.y - p1.rel.y) / (p2.rel.z - p1.rel.z);

  temp.x = p1.rel.x + dx * (z - p1.rel.z);
  temp.y = p1.rel.y + dy * (z - p1.rel.z);
  temp.z = z;

  return temp;
}


inline point_3d clip_edge_lr (point_3d p1, point_3d p2, double view_m)
{
  point_3d temp;

  if (is_approx_zero (p1.z - p2.z)) {
    temp.x = view_m * p1.z;
    temp.z = p1.z;
    double m = (p2.y - p1.y) / (p2.x - p1.x);
    double b = p1.y - m * p1.x;
    temp.y = m * temp.x + b;
  }
  else
  {
    line_3d l3d = calc_line_3d (p1, p2);
    temp.z = l3d.bx / (view_m - l3d.mx);
    temp.x = l3d.mx * temp.z + l3d.bx;
    temp.y = l3d.my * temp.z + l3d.by;
  }
  
  return temp;
}


inline point_3d clip_edge_tb (point_3d p1, point_3d p2, double view_m)
{
  point_3d temp;

  if (is_approx_zero (p1.z - p2.z)) {
    temp.y = view_m * p1.z;
    temp.z = p1.z;
    double m = (p2.x - p1.x) / (p2.y - p1.y);
    double b = p1.x - m * p1.y;
    temp.x = m * temp.y + b;
  }
  else
  {
    line_3d l3d = calc_line_3d (p1, p2);
    temp.z = l3d.by / (view_m - l3d.my);
    temp.x = l3d.mx * temp.z + l3d.bx;
    temp.y = l3d.my * temp.z + l3d.by;
  }
  
  return temp;
}


inline line_3d calc_line_3d (point_3d p1, point_3d p2)
{
  line_3d l3d;
  
  l3d.mx = (p2.x - p1.x) / (p2.z - p1.z);
  l3d.my = (p2.y - p1.y) / (p2.z - p1.z);
  l3d.bx = p1.x - l3d.mx * p1.z;
  l3d.by = p1.y - l3d.my * p1.z;

  return l3d;
}


void show_gfx (view_type* view)
{
  int page, rem, p;
  int source;
  int dest_offset;
  int y;
  unsigned char* temp = (unsigned char*)_x2_buff;
 
//  if (view->vis_page)
//    page = 0;
//  else
    page = 0;
  
//  while (inp(0x3DA) & 0x08);
//  while (!(inportb(0x3DA) & 0x08));

  __djgpp_nearptr_enable();
  source = (int)temp;
  
  y = 0;
  set_vesa_bank(0);
  for (dest_offset = 0; dest_offset < 65536 - 640 * 3; dest_offset += 640 * 3) {
    write_row_to_scrn (source, 320 * 3 * page + dest_offset, 320 * 3);
    source += 320 * 3;
    y++;
  }

  for (p = 1; p <= 4; p++) {
    int rem = (p * 65536) - (3 * (2 * 320 * y));
    write_row_to_scrn (source, 320 * 3 * page + dest_offset, rem);
    set_vesa_bank(p);
    write_row_to_scrn (rem + int(temp) + 3 * (y * 320), 320 * 3 * page, 320 * 3 - rem);
    y++;
    source = int(temp) + 3 * (y * 320);
    dest_offset = (3 * (2 * 320 * y)) & 65535;
    for (; dest_offset < 65536 - 640 * 3; ) {
      write_row_to_scrn (source, 320 * 3 * page + dest_offset, 320 * 3);
      source += 320 * 3;
      dest_offset += 640 * 3;
      y++;
    }
  }
/*
  rem = 2 * 65536 - (3 * (2 * 320 * y));
  write_row_to_scrn (source, dest_offset, rem);

  set_vesa_bank(2);
  write_row_to_scrn (rem + int(temp) + 3 * (y * 320), 0, 320 * 3 - rem);
  y++;  
  source = int(temp) + 3 * (y * 320);
  dest_offset = (3 * (2 * 320 * y)) & 65535;
  for (; dest_offset < 65536 - 640 * 3; ) {
    write_row_to_scrn (source, dest_offset, 320 * 3);
    source += 320 * 3;
    dest_offset += 640 * 3;
    y++;
  }
  rem = 3 * 65536 - (3 * (2 * 320 * y));
  write_row_to_scrn (source, dest_offset, rem);

  set_vesa_bank(3);
  write_row_to_scrn (rem + int(temp) + 3 * (y * 320), 0, 320 * 3 - rem);
  y++;  
  source = int(temp) + 3 * (y * 320);
  dest_offset = (3 * (2 * 320 * y)) & 65535;
  for (; dest_offset < 65536 - 640 * 3; ) {
    write_row_to_scrn (source, dest_offset, 320 * 3);
    source += 320 * 3;
    dest_offset += 640 * 3;
    y++;
  }

  y++;
  set_vesa_bank(4);
  source = int(temp) + 3 * (y * 320);
  dest_offset = (3 * (2 * 320 * y)) & 65535;
  for (; dest_offset < 65536 - 640 * 3; ) {
    write_row_to_scrn (source, dest_offset, 320 * 3);
    source += 320 * 3;
    dest_offset += 640 * 3;
    y++;
  }
*/

  rem = (p * 65536) - (3 * (2 * 320 * y));
  write_row_to_scrn (source, 320 * 3 * page + dest_offset, rem);
  set_vesa_bank(p);
  write_row_to_scrn (rem + int(temp) + 3 * (y * 320), 320 * 3 * page, 320 * 3 - rem);
  y++;
  source = int(temp) + 3 * (y * 320);
  dest_offset = (3 * (2 * 320 * y)) & 65535;

  for (; y < 200; ) {
    write_row_to_scrn (source, 320 * 3 * page + dest_offset, 320 * 3);
    source += 320 * 3;
    dest_offset += 640 * 3;
    y++;
  }

  __djgpp_nearptr_disable();

/*
  if (view->vis_page) {
    set_first_pxl (0, 0);
    page = 0;
    view->vis_page == 0;
  }
  else {
    set_first_pxl (WIDTH, 0);
    page = 1;
    view->vis_page == 1;
  }
*/
}


void write_row_to_scrn (int source, int dest_offset, int len)
{
  asm volatile (
    "movl %0, %%esi \n"
    "movl $0xA0000, %%edi\n"
    "addl %1, %%edi \n"
    "subl ___djgpp_base_address, %%edi \n"
    "movl %2, %%ecx \n"
    "subl $4, %%ecx \n"
    ".align 4 \n"
    
    "write_row_to_scrn_l1: \n"
    "  movl (%%esi, %%ecx), %%eax \n"
    "  movl %%eax, (%%edi, %%ecx) \n"
    "sub $4, %%ecx \n"
    "jnl write_row_to_scrn_l1 \n"
  :
  :"g" (source), "g" (dest_offset), "g" (len)
  :"eax", "ecx", "edi", "esi", "memory"
  );
}


void pxl (int x, int y, int blue, int green, int red)
{
 unsigned char* temp = (unsigned char*)_x2_buff;
 
 if ((x >= 0) && (y >= 0) && (x <= WIDTH - 1) && (y <= HEIGHT - 1)) {
   //_farpokeb (_dos_ds, 0xA0000 + x + y * WIDTH, color);
   temp[0 + 3 * (x + y * WIDTH)] = blue;
   temp[1 + 3 * (x + y * WIDTH)] = green + _test_color;
   temp[2 + 3 * (x + y * WIDTH)] = red;
 }
// else {
//   printf ("out of bounds %i, %i\n", x, y);
//   getch();
// }         //_farpokeb (_dos_ds, 0xA0000 + x + y * WIDTH, color);
}


void pxl (point_2d p, int blue, int green, int red)
{
 unsigned char* temp = (unsigned char*)_x2_buff;
 int x = (int)p.x;
 int y = (int)p.y;
 
 if ((x >= 0) && (y >= 0) && (x <= WIDTH - 1) && (y <= HEIGHT - 1)) {
   //_farpokeb (_dos_ds, 0xA0000 + x + y * WIDTH, color);
   temp[0 + 3 * (x + y * WIDTH)] = blue;
   temp[1 + 3 * (x + y * WIDTH)] = green + _test_color;
   temp[2 + 3 * (x + y * WIDTH)] = red;
 }
// else {
//   printf ("out of bounds %i, %i\n", x, y);
//   getch();
// }         //_farpokeb (_dos_ds, 0xA0000 + x + y * WIDTH, color);
}


double inline calc_inf()
{
  double a = 1, b = 0;

  _inf_double = double(a / b);
  _inf_float = float(a / b);
  _nan_double = double(b / b);
  _nan_float = float(b / b);
}


/*
inline void calc_sect_lr
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  double inv_12 = 1 / (point[1].rel.z - point[2].rel.z);
  double inv_03 = 1 / (point[0].rel.z - point[3].rel.z);

  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x;
    texdat[s].ind2 = *rig_x;
    texdat[s].u1 = 0;
    texdat[s].u2 = bmp.width;
    texdat[s].v1 = bmp.height *
      (point[1].rel.z - z) * inv_12;
    texdat[s].v2 = bmp.height *
      ((point[0].rel.z) - z) * inv_03;
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) * texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) * texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_tb
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  double inv_01 = 1 / (point[0].rel.z - point[1].rel.z);
  double inv_32 = 1 / (point[3].rel.z - point[2].rel.z);

  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].v1 = 0;
    texdat[s].v2 = bmp.height;
    texdat[s].u1 = bmp.width *
      (point[0].rel.z - z) * inv_01;
    texdat[s].u2 = bmp.width *
      (point[3].rel.z - z) * inv_32;
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_bt
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].u2 = bmp.width *
      (((point[0].rel.z) - z) / ((point[0].rel.z) - (point[1].rel.z)));
    texdat[s].u1 = bmp.width *
      (((point[3].rel.z) - z) / ((point[3].rel.z) - (point[2].rel.z)));
    texdat[s].v2 = 0;
    texdat[s].v1 = bmp.height;
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_rl
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].u2 = .000;
    texdat[s].u1 = bmp.width - .000;
    texdat[s].v2 = .00 + (bmp.height - .00) *
      (((point[1].rel.z) - z) / ((point[1].rel.z) - (point[2].rel.z)));
    texdat[s].v1 = 0 + bmp.height * 1 *
      (((point[0].rel.z) - z) / ((point[0].rel.z) - (point[3].rel.z)));
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_lt
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  double inv_01 = 1 / (point[0].rel.z - point[1].rel.z);
  double inv_03 = 1 / (point[0].rel.z - point[3].rel.z);

  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].u1 = 0;
    texdat[s].u2 = bmp.width *
      (point[0].rel.z - z) * inv_01;
    texdat[s].v1 = bmp.height *
      (point[0].rel.z - z) * inv_03;
    texdat[s].v2 = 0;
//    printf ("%f, ", lef_dx);
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_tl
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].u2 = 0;
    texdat[s].u1 = bmp.width *
      ((point[0].rel.z - z) / (point[0].rel.z - point[1].rel.z));
    texdat[s].v2 = bmp.height *
      ((point[0].rel.z - z) / (point[0].rel.z - point[3].rel.z));
    texdat[s].v1 = 0;
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_tr
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x;
    texdat[s].ind2 = *rig_x;
    texdat[s].u1 = (bmp.width - .0)  *
      ((point[0].rel.z - z) / (point[0].rel.z - point[1].rel.z));
    texdat[s].u2 = bmp.width;
    texdat[s].v1 = .001;
    texdat[s].v2 = (bmp.height - .0)  *
      ((point[1].rel.z - z) / (point[1].rel.z - point[2].rel.z));
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_rt
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .0000;
    texdat[s].ind2 = *rig_x + .0000;
    texdat[s].u2 = bmp.width *
      ((point[0].rel.z - z) / (point[0].rel.z - point[1].rel.z));
    texdat[s].u1 = bmp.width;
    texdat[s].v2 = 0;
    texdat[s].v1 = bmp.height *
      ((point[1].rel.z - z) / (point[1].rel.z - point[2].rel.z));
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_br
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].u1 = bmp.width *
      ((point[3].rel.z - z) / (point[3].rel.z - point[2].rel.z));
    texdat[s].u2 = bmp.width;
    texdat[s].v1 = bmp.height;
    texdat[s].v2 = bmp.height *
      ((point[1].rel.z - z) / (point[1].rel.z - point[2].rel.z));
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_rb
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].u2 = bmp.width *
      ((point[3].rel.z - z) / (point[3].rel.z - point[2].rel.z));
    texdat[s].u1 = bmp.width;
    texdat[s].v2 = bmp.height;
    texdat[s].v1 = bmp.height *
      ((point[1].rel.z - z) / (point[1].rel.z - point[2].rel.z));
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_bl
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].u1 = bmp.width *
      ((point[3].rel.z - z) / (point[3].rel.z - point[2].rel.z));
    texdat[s].u2 = 0;
    texdat[s].v1 = bmp.height;
    texdat[s].v2 = bmp.height *
      ((point[0].rel.z - z) / (point[0].rel.z - point[3].rel.z));
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -99;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}


inline void calc_sect_lb
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double *lef_x, double *rig_x, double lef_dx, double rig_dx,
   double scan, double scan_end, double scan_m, point_type* point,
   double k, double c, double m, double b, double* p, double dp)
{
  for (int s = DAT_MID + (int)floor(scan) - 1; s <= DAT_MID + (int)ceil(scan_end) + 1; s++) {
    double z = (k * b) / (*p - c - k * m);
    texdat[s].ind1 = *lef_x - .000;
    texdat[s].ind2 = *rig_x + .000;
    texdat[s].u2 = bmp.width *
      ((point[3].rel.z - z) / (point[3].rel.z - point[2].rel.z));
    texdat[s].u1 = 0;
    texdat[s].v2 = bmp.height;
    texdat[s].v1 = bmp.height *
      ((point[0].rel.z - z) / (point[0].rel.z - point[3].rel.z));
    texdat[s].inv_len = 1 / (*rig_x - *lef_x);
    texdat[s].du = cv_sng(
      (texdat[s].u2 - texdat[s].u1) *
    texdat[s].inv_len);
    texdat[s].dv = cv_sng(
      (texdat[s].v2 - texdat[s].v1) *
    texdat[s].inv_len);
    texdat[s].last_ind = -1;
    *lef_x += lef_dx;
    *rig_x += rig_dx;
    *p += dp;
  }
}
*/


void gfx_h_line (int y, int x1, int x2, double scan_m, int color,
  view_type view)
{
  if (x1 > x2)
    return;
    
/*
  if (x1 < 0)
    x1 = 0;
  else if (x1 > 319)
    x1 = 319;
    
  if (x2 < 0)
    x2 = 0;
  else if (x2 > 319)
    x2 = 319;
*/

  sng m = cv_sng(scan_m);
  sng scan = cv_sng(x1 + .5 - scan_m * (y + .5) + DAT_MID);
  int s = cv_int(scan);
  sng f = (scan - cv_sng(s));
  double sca = (x1 + .5 - scan_m * (y + .5));
  double fra = sca - floor(x1 + .5 - scan_m * (y + .5));
  _f = (unsigned long)(fra * (double)65536);

  double etop1, ebot1, etop2, ebot2, inv_len1, inv_len2;
  int x;
  double fy = double(y) + .5;
  _dtemp[1] = double(y) + .5;
  
///*
//  if (y % 32 == 0) //{
//    for (x = x1; x <= x2 + 1; x++, s++)
//      update_uv(s, fy, int(&_texdat[s].u));
//  else {

//    if (view.z_map[x1 + y * 320].poly == _poly)
//      if (_texdat[s].last_ind != y) //{
//        update_uv(s, fy, int(&_texdat[s].u));

//    for (x = x1; x <= x2 + 1; x++, s++)
//      if (view.z_map[x + y * 320].poly == _poly) {
//        if (_texdat[s + 0].last_ind != y) //{
//          update_uv(s, fy, int(&_texdat[s + 0].u));
//        if (_texdat[s + 1].last_ind != y) //{
//          update_uv(s, fy, int(&_texdat[s + 1].u));
//      }
 //  }

//*/

s = cv_int(scan);
//update_uv(s, fy, int(&_texdat[s].u));
//_texdat[s].last_ind = y + 1;

int yc = y * WIDTH;
_temp3 = y + 1;

_temp4 = s * sizeof(texdat_type);
_scrn = (x1 + yc) * 3;
_temp2 = x2 - x1;
_temp5 = (int)_bmp.texel;
int sc = x1 + yc;
_temp8 = (int)(&view.z_map[sc].poly);
_temp11 = y;

//  _temp19 = (int)_shad.texel + 3 * ((64 - (int(_duv.x) & 63)) + 128 * (64 - (int(_duv.y) & 63)));
  _temp19 = (int)_shad.texel;
  _temp21 = long(65536 * (64 - _duv.x));
  _temp22 = long(65536 * (64 - _duv.y));
  
  if (_seg_mode == 0)
    gfx_h_seg1 (y, x1, x2, m, color, view);
  else if (_seg_mode == 1)
    gfx_h_seg2 (y, x1, x2, scan_m, color, view);
  else //if (_seg_mode == 2)
    draw_solid_horz_line (x1, x2, y, color);
}


void gfx_h_seg1 (int y, int x1, int x2, sng scan_m, int color,
  view_type view)
{
  asm volatile (
    "movl %%edi, __edi \n"
    "movl %%ebp, __ebp \n"
    "movl %%esp, __esp \n"
    "movl %%esi, __esi \n"
    "movl __x2_buff, %%edi \n"
    "addl __scrn, %%edi \n"
    "movl __f, %%esi \n"
    "leal __texdat, %%ebp \n"
    "addl __temp4, %%ebp \n"
//    "movl __poly, %%edx \n"
    "movl __temp8, %%ebx \n"

  "l_gfx_h_seg11: \n"
    "movl 0 + 16(%%ebp), %%eax \n"
    "movl __temp11, %%ecx \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_hseg12 \n"
      "fldl __dtemp + 8 * 1 \n" // load y
      "fldl 0 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 0 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 0 + 20(%%ebp) \n"
      "fldl 0 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 0(%%ebp) \n"

      "fldl 0 + 28(%%ebp) \n"
      "fldl 0 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 4(%%ebp) \n"
      "movl %%ecx, 0 + 16(%%ebp) \n"
    "s_gfx_hseg12: \n"

    "movl 88 + 16(%%ebp), %%eax \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_hseg13 \n"

      "fldl __dtemp + 8 * 1 \n" // load y
      "fldl 88 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 88 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 88 + 20(%%ebp) \n"
      "fldl 88 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 0(%%ebp) \n"

      "fldl 88 + 28(%%ebp) \n"
      "fldl 88 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 4(%%ebp) \n"
      "movl %%ecx, 88 + 16(%%ebp) \n"
    "s_gfx_hseg13: \n"

    "movl %%ebx, __temp7 \n"
    "movl 0(%%ebp), %%ecx \n"
    "movl 4(%%ebp), %%esp \n"
    "movl 8(%%ebp), %%eax \n"
    "movl 12(%%ebp), %%ebx \n"
    "addl %%ecx, %%eax \n"
    "addl %%esp, %%ebx \n"
    "movl %%eax, 0(%%ebp) \n"
    "movl %%ebx, 4(%%ebp) \n"
    "movl 88 + 0(%%ebp), %%eax \n"
    "movl 88 + 4(%%ebp), %%ebx \n"
    "subl %%ecx, %%eax \n"
    "subl %%esp, %%ebx \n"

    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%edx, %%ecx \n"

    "movl %%ebx, %%eax \n"
    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%esp, %%edx \n"

    "andl $63 << 16, %%ecx \n"
    "andl $63 << 16, %%edx \n"
    "shrl $16, %%ecx \n"
    "shrl $16 - 6, %%edx \n"
    "addl %%ecx, %%edx \n"
    "movl __temp5, %%ebx \n"
    "leal (%%edx, %%edx, 2), %%edx \n"
    "movl __temp3, %%eax \n"

//    "xorl %%ecx, %%ecx \n"

    "movl __temp16, %%ecx \n"
    "movb 0(%%ebx, %%edx), %%cl \n"    
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 0(%%edi) \n"
    
    "movl __temp17, %%ecx \n"
    "movb 1(%%ebx, %%edx), %%cl \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 1(%%edi) \n"
    
    "movl __temp18, %%ecx \n"
    "movb 2(%%ebx, %%edx), %%cl \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 2(%%edi) \n"

    "movl %%eax, 16(%%ebp) \n"
    "movl __temp7, %%ebx \n"

    "s_gfx_h_seg11: \n"

    "addl $8, %%ebx \n"
    "addl $3, %%edi \n"
    "addl $88, %%ebp \n"
  "decl __temp2 \n"
  "jge l_gfx_h_seg11 \n"

  "movl __esp, %%esp \n"
  "movl __ebp, %%ebp \n"
  "movl __edi, %%edi \n"
  "movl __esi, %%esi \n"
  :
  :"eax" (_temp4), "g" (_bmp.texel), "g" (x2), "g" (y), "g" (scan_m)
  :"memory", "ebx", "ecx", "edx", "esi"
  );
}


void gfx_h_seg2 (int x, int y1, int y2, double scan_m, int color,
  view_type view)
{
  asm volatile (
    "push %%edi \n"
    "movl __x2_buff, %%edi \n"
    "addl __scrn, %%edi \n"
    "movl __f, %%esi \n"
    "push %%ebp \n"
    "movl %%esp, __temp \n"
    "leal __texdat, %%ebp \n"
    "addl __temp4, %%ebp \n"
//    "movl __poly, %%edx \n"
    "movl __temp8, %%ebx \n"

  "l_gfx_h_seg21: \n"

//    "movl (%%ebx), %%eax \n"
//    "cmpl %%eax, __poly \n"
//    "jne s_gfx_h_seg21 \n"

    "movl 0 + 16(%%ebp), %%eax \n"
    "movl __temp11, %%ecx \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_hseg22 \n"
      "fldl __dtemp + 8 * 1 \n" // load y
      "fldl 0 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 0 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 0 + 20(%%ebp) \n"
      "fldl 0 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 0(%%ebp) \n"

      "fldl 0 + 28(%%ebp) \n"
      "fldl 0 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 4(%%ebp) \n"
      "movl %%ecx, 0 + 16(%%ebp) \n"
    "s_gfx_hseg22: \n"

    "movl 88 + 16(%%ebp), %%eax \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_hseg23 \n"

      "fldl __dtemp + 8 * 1 \n" // load y
      "fldl 88 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 88 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 88 + 20(%%ebp) \n"
      "fldl 88 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 0(%%ebp) \n"

      "fldl 88 + 28(%%ebp) \n"
      "fldl 88 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 4(%%ebp) \n"
      "movl %%ecx, 88 + 16(%%ebp) \n"
    "s_gfx_hseg23: \n"

    "movl %%ebx, __temp7 \n"
    "movl 0(%%ebp), %%ecx \n"
    "movl 4(%%ebp), %%esp \n"
    "movl 8(%%ebp), %%eax \n"
    "movl 12(%%ebp), %%ebx \n"
    "addl %%ecx, %%eax \n"
    "addl %%esp, %%ebx \n"
    "movl %%eax, 0(%%ebp) \n"
    "movl %%ebx, 4(%%ebp) \n"
    "movl 88 + 0(%%ebp), %%eax \n"
    "movl 88 + 4(%%ebp), %%ebx \n"
    "subl %%ecx, %%eax \n"
    "subl %%esp, %%ebx \n"

    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%edx, %%ecx \n" // ecx = u (16.16)

    "movl %%ebx, %%eax \n"
    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%esp, %%edx \n" // edx = v (16.16)

    "movl %%ecx, %%esp \n"
    "movl %%edx, %%eax \n"
    "addl __temp21, %%esp \n"
    "addl __temp22, %%eax \n"
    "andl $127 << 16, %%esp \n"
    "andl $127 << 16, %%eax \n"
    "shrl $16, %%esp \n"
    "shrl $16 - 7, %%eax \n"
    "addl %%esp, %%eax \n"
    "leal (%%eax, %%eax, 2), %%eax \n"
    "addl __temp19, %%eax \n"

    "shrl $16, %%ecx \n"
//    "movl %%ecx, %%esp \n"
//    "andl $127, %%esp \n"
//    "movl %%edx, %%eax \n"
//    "andl $127 << 16, %%eax \n"
//    "shrl $16 - 7, %%eax \n"
//    "addl %%esp, %%eax \n"
//    "leal (%%eax, %%eax, 2), %%eax \n"
//    "addl __temp19, %%eax \n"

    "andl $63 << 16, %%edx \n"
    "andl $63 << 0, %%ecx \n"
    "shrl $16 - 6, %%edx \n"

    "addl %%ecx, %%edx \n"
    "movl __temp5, %%ebx \n"
    "leal (%%edx, %%edx, 2), %%edx \n"
    "movl __temp3, %%esp \n"

//    "xorl %%ecx, %%ecx \n"

//    "movl (%%eax), %%eax \n"
//    "addl __temp19, %%eax \n"
    
    "movl __temp16, %%ecx \n"
    "movl 0(%%eax), %%esp \n"
    "andl $255, %%esp \n"
    "movb 0(%%ebx, %%edx), %%cl \n"
    "shll $8, %%esp \n"
    "addl %%esp, %%ecx \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 0(%%edi) \n"
    
    "movl __temp17, %%ecx \n"
    "movl 1(%%eax), %%esp \n"
    "andl $255, %%esp \n"
    "movb 1(%%ebx, %%edx), %%cl \n"
    "shll $8, %%esp \n"
    "addl %%esp, %%ecx \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 1(%%edi) \n"
    
    "movl __temp18, %%ecx \n"
    "movl 2(%%eax), %%esp \n"
    "andl $255, %%esp \n"    
    "movb 2(%%ebx, %%edx), %%cl \n"
    "shll $8, %%esp \n"
    "addl %%esp, %%ecx \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 2(%%edi) \n"

    "movl __temp17, %%esp \n"
    "movl %%esp, 16(%%ebp) \n"
    "movl __temp7, %%ebx \n"

    "s_gfx_h_seg21: \n"

    "addl $8, %%ebx \n"
    "addl $3, %%edi \n"
    "addl $88, %%ebp \n"
  "decl __temp2 \n"
  "jge l_gfx_h_seg21 \n"

    "movl __temp, %%esp \n"
    "pop %%ebp \n"
    "pop %%edi \n"
  :
  :"eax" (_temp4), "g" (_bmp.texel)
  :"memory", "ebx", "ecx", "edx", "esi"
  );
}


void gfx_v_line4 (int y, int x1, int x2, double scan_m, int color,
  view_type view)
{
  if (x1 > x2)
    return;
    
  sng m = cv_sng(scan_m);
  sng scan = cv_sng(y + .5 - scan_m * (x1 + .5) + DAT_MID);
  int s = cv_int(scan);
  sng f = (scan - cv_sng(s));
  double sca = (y + .5 - scan_m * (x1 + .5));
  double fra = sca - floor(y + .5 - scan_m * (x1 + .5));
  _f = (unsigned long)(fra * (double)65536);

  double etop1, ebot1, etop2, ebot2, inv_len1, inv_len2;
  int x;
  double fy = double(y) + .5;
  _dtemp[1] = double(y) + .5;

s = cv_int(scan);

int yc = y * WIDTH;
_temp3 = y + 1;

_temp4 = s * sizeof(texdat_type);
_scrn = (x1 + yc) * 3;
_temp2 = x2 - x1;
_temp5 = (int)_bmp.texel;
int sc = x1 + yc;
_temp8 = (int)(&view.z_map[sc].poly);
_temp11 = y;

//  _temp19 = (int)_shad.texel + 3 * ((64 - (int(_duv.x) & 63)) + 128 * (64 - (int(_duv.y) & 63)));
  _temp19 = (int)_shad.texel;
  _temp21 = long(65536 * (64 - _duv.x));
  _temp22 = long(65536 * (64 - _duv.y));

  _tri_data.count = x2 - x1;
  _tri_data.back_r = _temp15;
  _tri_data.back_g = _temp16;
  _tri_data.back_b = _temp17;
  _tri_data.bmp.texel = _bmp.texel;

  gfx_v_line_pos (y, x1, x2, scan_m, color, view);
  
//  update_uv (s, x1);
//  draw_gfx_v_seg((int)_x2_buff + (x1 + y * 320) * 3, int(&_texdat[s]));
}


/*
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
*/


void draw_gfx_v_seg (long start, long array)
{
  asm volatile (
    "draw_gfx_v_seg_start_1: \n"
    "push %%ebp \n"
    "movl %0, %%edi \n"
    "movl %1, %%ebp \n"
    "movl %%esp, __esp \n"
    "movl $0, %%esi \n"

    "draw_gfx_v_seg_rep_1: \n"
    "  movl 0(%%ebp), %%ecx \n"
    "  movl 4(%%ebp), %%esp \n"
    "  movl 8(%%ebp), %%eax \n"
    "  movl 12(%%ebp), %%ebx \n"
    "  addl %%ecx, %%eax \n"
    "  addl %%esp, %%ebx \n"
    "  movl %%eax, 0(%%ebp) \n"
    "  movl %%ebx, 4(%%ebp) \n"
    "  movl 88 + 0(%%ebp), %%eax \n"
    "  movl 88 + 4(%%ebp), %%ebx \n"
    "  subl %%ecx, %%eax \n"
    "  subl %%esp, %%ebx \n"

    "  imull %%esi \n"
    "  shll $16, %%edx \n"
    "  shrl $16, %%eax \n"
    "  addl %%eax, %%edx \n"
    "  addl %%edx, %%ecx \n"

    "  movl %%ebx, %%eax \n"
    "  imull %%esi \n"
    "  shll $16, %%edx \n"
    "  shrl $16, %%eax \n"
    "  addl %%eax, %%edx \n"
    "  addl %%esp, %%edx \n"

    "  andl $63 << 16, %%ecx \n"
    "  andl $63 << 16, %%edx \n"
    "  shrl $16, %%ecx \n"
    "  shrl $16 - 6, %%edx \n"
    "  addl %%ecx, %%edx \n"
    "  movl __tri_data + 28, %%ebx \n"
    "  leal (%%edx, %%edx, 2), %%edx \n"

    "  movl __tri_data + 16, %%ecx \n"
    "  movb 0(%%ebx, %%edx), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 0(%%edi) \n"
    
    "  movl __tri_data + 20, %%ecx \n"
    "  movb 1(%%ebx, %%edx), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 1(%%edi) \n"
    
    "  movl __tri_data + 24, %%ecx \n"
    "  movb 2(%%ebx, %%edx), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 2(%%edi) \n"
    
    "  addl $3, %%edi \n"
//    "  addl $88, %%ebp \n"
    "decl __tri_data + 0 \n"
    "jge draw_gfx_v_seg_rep_1 \n"

    "movl __esp, %%esp \n"
    "pop %%ebp \n"
    :
    :"g" (start), "g" (array)
    :"eax", "ebx", "ecx", "edx", "esi", "edi"
  );
}


/*
void gfx_v_line_neg2 (int y, int x1, int x2, double scan_m, int color,
  view_type view)
{
  if (x1 > x2)
    return;

  _tri_data.back_r = 128 << 8;
  _tri_data.back_g = 128 << 8;
  _tri_data.back_b = 128 << 8;
  _tri_data.bmp.texel = _bmp.texel;

  sng m = cv_sng(scan_m);
  long s = int(65536 * (double(y + .5) - double(x1 + .5) * scan_m));
  int scrn = (unsigned int)_x2_buff + (unsigned int)3 * ((unsigned int)x1 + (unsigned int)y * (unsigned int)320);
  _temp3 = x1;
  _temp4 = x2 + 1;//int(&_texdat[DAT_MID + s_scan.whole]);
  _temp5 = m.valu;
  _temp6 = s & 65535;
  _temp7 = int(&_texdat[DAT_MID + (s >> 16)]);
  
//  while (_temp3 <= x2) {
//    int scan = s_scan.whole;//(int)floor(f_scan);

    asm (
      "push %%ebp \n"
      "push %%esi \n"
      "movl %%esp, __esp \n"
      "movl %1, %%esi \n"
      "movl %3, %%edi \n"
      "movl %0, %%ebp \n"

      // start update u & v
      "gfx_v_line_neg_rep1: \n"
      "  movl 16(%%ebp), %%eax \n"
      "  movl __temp3, %%edx \n"
      "  cmp %%eax, %%edx \n"
      "  je gfx_v_line_neg_skip1 \n"
      "    fild __temp3 \n" // load x
      "    fldl __half \n"
      "    faddp %%st(0), %%st(1) \n"
      "    fldl 0 + 52(%%ebp) \n"
      "    fsubrp %%st(0), %%st(1) \n"
      "    fldl 0 + 68(%%ebp) \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fstpl __dtemp + 8 * 0 \n"

      "    fldl 0 + 20(%%ebp) \n"
      "    fldl 0 + 36(%%ebp) \n"
      "    fsubp %%st(0), %%st(1) \n"
      "    fldl __dtemp + 0 * 8 \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fldl 0 + 20(%%ebp) \n"
      "    faddp %%st(0), %%st(1) \n"
      "    fldl __f2sng \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fldl __half \n"
      "    fsubrp %%st(0), %%st(1) \n"
      "    fistpl 0 + 0(%%ebp) \n"

      "    fldl 0 + 28(%%ebp) \n"
      "    fldl 0 + 44(%%ebp) \n"
      "    fsubp %%st(0), %%st(1) \n"
      "    fldl __dtemp + 0 * 8 \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fldl 0 + 28(%%ebp) \n"
      "    faddp %%st(0), %%st(1) \n"
      "    fldl __f2sng \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fldl __half \n"
      "    fsubrp %%st(0), %%st(1) \n"
      "    fistpl 0 + 4(%%ebp) \n"

      "    fild __temp3 \n" // load x
      "    fldl __half \n"
      "    faddp %%st(0), %%st(1) \n"
      "    fldl 88 + 52(%%ebp) \n"
      "    fsubrp %%st(0), %%st(1) \n"
      "    fldl 88 + 68(%%ebp) \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fstpl __dtemp + 8 * 0 \n"

      "    fldl 88 + 20(%%ebp) \n"
      "    fldl 88 + 36(%%ebp) \n"
      "    fsubp %%st(0), %%st(1) \n"
      "    fldl __dtemp + 0 * 8 \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fldl 88 + 20(%%ebp) \n"
      "    faddp %%st(0), %%st(1) \n"
      "    fldl __f2sng \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fldl __half \n"
      "    fsubrp %%st(0), %%st(1) \n"
      "    fistpl 76(%%ebp) \n"

      "    fldl 88 + 28(%%ebp) \n"
      "    fldl 88 + 44(%%ebp) \n"
      "    fsubp %%st(0), %%st(1) \n"
      "    fldl __dtemp + 0 * 8 \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fldl 88 + 28(%%ebp) \n"
      "    faddp %%st(0), %%st(1) \n"
      "    fldl __f2sng \n"
      "    fmulp %%st(0), %%st(1) \n"
      "    fldl __half \n"
      "    fsubrp %%st(0), %%st(1) \n"
      "    fistpl 80(%%ebp) \n"
      "  gfx_v_line_neg_skip1: \n"
      // end update u & v
      
      "  movl 0(%%ebp), %%ebx \n"
      "  movl 8(%%ebp), %%ecx \n"
      "  movl 76(%%ebp), %%eax \n"
      "  movl 8+88(%%ebp), %%esp \n"
      "addl %%ebx, %%ecx \n"
      "addl %%eax, %%esp \n"
      "movl %%ecx, 0(%%ebp) \n"
      "movl %%esp, 76(%%ebp) \n"

      "subl %%ebx, %%eax \n"
      "imull %%esi \n"
      "shll $16, %%edx \n"
      "shrl $16, %%eax \n"
      "addl %%eax, %%edx \n"
      "addl %%edx, %%ebx \n"
      "shrl $16, %%ebx \n"
      "andl $63, %%ebx \n"
      "movl %%ebx, __temp1 \n"

      "movl 4(%%ebp), %%ebx \n"
      "movl 12(%%ebp), %%ecx \n"
      "movl 80(%%ebp), %%eax \n"
      "movl 12+88(%%ebp), %%esp \n"
      "addl %%ebx, %%ecx \n"
      "addl %%eax, %%esp \n"
      "movl %%ecx, 4(%%ebp) \n"
      "movl %%esp, 80(%%ebp) \n"

      "subl %%ebx, %%eax \n"
      "imull %%esi \n"
      "shll $16, %%edx \n"
      "shrl $16, %%eax \n"
      "addl %%eax, %%edx \n"
      "addl %%edx, %%ebx \n"
      "andl $63 << 16, %%ebx \n"
      "shrl $16 - 6, %%ebx \n"
      "movl __temp1, %%eax \n"
      "addl %%ebx, %%eax \n"
      "leal (%%eax, %%eax, 2), %%eax \n"
      "movl __tri_data + 28, %%ebx \n"

    "  movl __tri_data + 16, %%ecx \n"
    "  movb 0(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 0(%%edi) \n"
    
    "  movl __tri_data + 20, %%ecx \n"
    "  movb 1(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 1(%%edi) \n"
    
    "  movl __tri_data + 24, %%ecx \n"
    "  movb 2(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 2(%%edi) \n"

//      "movb 0(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 0(%%edi) \n"
//      "movb 1(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 1(%%edi) \n"
//      "movb 2(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 2(%%edi) \n"

      "addl $3, %%edi \n"

      "movl __temp3, %%eax \n"
      "incl %%eax \n"
      "movl %%eax, 16(%%ebp) \n"
      "movl %%eax, __temp3 \n"
      
      "subw __temp5, %%si \n"
      "jnc gfx_v_line_neg_skip2 \n"
      "  addl $88, %%ebp \n"
      "gfx_v_line_neg_skip2: \n"

      "cmp __temp4, %%eax \n"
      "jl gfx_v_line_neg_rep1 \n"
      
      "movl __esp, %%esp \n"
      "pop %%esi \n"
      "pop %%ebp \n"
    :
    :"g" (_temp7), "g" (_temp6), "g" (_bmp.texel), "g" (scrn)
    :"eax", "ebx", "ecx", "edx", "edi", "esi", "ebp", "memory"
    );

//    _texdat[DAT_MID + scan].last_ind = x + 1;
//    s -= m.valu;
//    scrn += 3;
//  }

}
*/


/*
void gfx_v_seg1 (int x, int y1, int y2, double scan_m, int color,
  view_type view)
{
  asm volatile (
    "push %%edi \n"
    "movl __x2_buff, %%edi \n"
    "addl __scrn, %%edi \n"
    "movl __f, %%esi \n"
    "push %%ebp \n"
    "movl %%esp, __temp \n"
    "leal __texdat, %%ebp \n"
    "addl __temp4, %%ebp \n"
//    "movl __poly, %%edx \n"
    "movl __temp8, %%ebx \n"

  "l_gfx_h_seg11: \n"

//    "movl (%%ebx), %%eax \n"
//    "cmpl %%eax, __poly \n"
//    "jne s_gfx_h_seg11 \n"

    "movl 0 + 16(%%ebp), %%eax \n"
    "movl __temp11, %%ecx \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_hseg12 \n"
      "fldl __dtemp + 8 * 1 \n" // load y
      "fldl 0 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 0 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 0 + 20(%%ebp) \n"
      "fldl 0 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 0(%%ebp) \n"

      "fldl 0 + 28(%%ebp) \n"
      "fldl 0 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 4(%%ebp) \n"
      "movl %%ecx, 0 + 16(%%ebp) \n"
    "s_gfx_hseg12: \n"

    "movl 88 + 16(%%ebp), %%eax \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_hseg13 \n"

      "fldl __dtemp + 8 * 1 \n" // load y
      "fldl 88 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 88 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 88 + 20(%%ebp) \n"
      "fldl 88 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 0(%%ebp) \n"

      "fldl 88 + 28(%%ebp) \n"
      "fldl 88 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 4(%%ebp) \n"
      "movl %%ecx, 88 + 16(%%ebp) \n"
    "s_gfx_hseg13: \n"

    "movl %%ebx, __temp7 \n"
    "movl 0(%%ebp), %%ecx \n"
    "movl 4(%%ebp), %%esp \n"
    "movl 8(%%ebp), %%eax \n"
    "movl 12(%%ebp), %%ebx \n"
    "addl %%ecx, %%eax \n"
    "addl %%esp, %%ebx \n"
    "movl %%eax, 0(%%ebp) \n"
    "movl %%ebx, 4(%%ebp) \n"
    "movl 88 + 0(%%ebp), %%eax \n"
    "movl 88 + 4(%%ebp), %%ebx \n"
    "subl %%ecx, %%eax \n"
    "subl %%esp, %%ebx \n"

    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%edx, %%ecx \n"

    "movl %%ebx, %%eax \n"
    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%esp, %%edx \n"

    "andl $63 << 16, %%ecx \n"
    "andl $63 << 16, %%edx \n"
    "shrl $16, %%ecx \n"
    "shrl $16 - 6, %%edx \n"
    "addl %%ecx, %%edx \n"
    "movl __temp5, %%ebx \n"
    "leal (%%edx, %%edx, 2), %%edx \n"
    "movl __temp3, %%eax \n"

//    "xorl %%ecx, %%ecx \n"

    "movl __temp16, %%ecx \n"
    "movb 0(%%ebx, %%edx), %%cl \n"    
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 0(%%edi) \n"
    
    "movl __temp17, %%ecx \n"
    "movb 1(%%ebx, %%edx), %%cl \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 1(%%edi) \n"
    
    "movl __temp18, %%ecx \n"
    "movb 2(%%ebx, %%edx), %%cl \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 2(%%edi) \n"

    "movl %%eax, 16(%%ebp) \n"
    "movl __temp7, %%ebx \n"

    "s_gfx_h_seg11: \n"

    "addl $8, %%ebx \n"
    "addl $3, %%edi \n"
    "addl $88, %%ebp \n"
  "decl __temp2 \n"
  "jge l_gfx_h_seg11 \n"

    "movl __temp, %%esp \n"
    "pop %%ebp \n"
    "pop %%edi \n"
  :
  :"eax" (_temp4), "g" (_bmp.texel)
  :"memory", "ebx", "ecx", "edx", "esi"
  );
}
*/

/*
struct texdat_type {
  sng u, v; // 0, 4
  sng du, dv; // 8, 12
  int last_ind; // 16
  double u1, v1; // 20, 28
  double u2, v2; // 36, 44
  double ind1, ind2; // 52, 60
  double inv_len; // 68
  sng next_u, next_v; // 76, 80
};
*/

void gfx_v_line_pos (int y, int x1, int x2, double scan_m, int color,
  view_type view)
{
  if (x1 > x2)
    return;

  _tri_data.count = x2 - x1;
  _tri_data.back_r = _temp15;
  _tri_data.back_g = _temp16;
  _tri_data.back_b = _temp17;
  _tri_data.bmp.texel = _bmp.texel;

  sng m = cv_sng(scan_m);
  long s = int(65536 * (double(y + .5) - double(x1 + .5) * scan_m));
  int scrn = (unsigned int)_x2_buff + (unsigned int)3 * ((unsigned int)x1 + (unsigned int)y * (unsigned int)320);
  _temp3 = x1;
  _temp4 = x2 + 1;//int(&_texdat[DAT_MID + s_scan.whole]);
  _temp5 = m.valu;
  _temp6 = s & 65535;
  _temp7 = int(&_texdat[DAT_MID + (s >> 16)]);
  
//  while (_temp3 <= x2) {
//    int scan = s_scan.whole;//(int)floor(f_scan);

    asm (
      "push %%ebp \n"
      "push %%esi \n"
      "movl %%esp, __esp \n"
      "movl %1, %%esi \n"
      "movl %3, %%edi \n"
      "movl %0, %%ebp \n"

      // start update u & v
      "gfx_v_line_pos_rep1: \n"
      "movl 16(%%ebp), %%eax \n"
      "movl __temp3, %%edx \n"
      "cmp %%eax, %%edx \n"
      "je gfx_v_line_pos_skip1 \n"
      "  fild __temp3 \n" // load x
      "  fldl __half \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl 0 + 52(%%ebp) \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fldl 0 + 68(%%ebp) \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fstpl __dtemp + 8 * 0 \n"

      "  fldl 0 + 20(%%ebp) \n"
      "  fldl 0 + 36(%%ebp) \n"
      "  fsubp %%st(0), %%st(1) \n"
      "  fldl __dtemp + 0 * 8 \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl 0 + 20(%%ebp) \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl __f2sng \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl __half \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fistpl 0 + 0(%%ebp) \n"

      "  fldl 0 + 28(%%ebp) \n"
      "  fldl 0 + 44(%%ebp) \n"
      "  fsubp %%st(0), %%st(1) \n"
      "  fldl __dtemp + 0 * 8 \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl 0 + 28(%%ebp) \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl __f2sng \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl __half \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fistpl 0 + 4(%%ebp) \n"

      "  fild __temp3 \n" // load x
      "  fldl __half \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl 88 + 52(%%ebp) \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fldl 88 + 68(%%ebp) \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fstpl __dtemp + 8 * 0 \n"

      "  fldl 88 + 20(%%ebp) \n"
      "  fldl 88 + 36(%%ebp) \n"
      "  fsubp %%st(0), %%st(1) \n"
      "  fldl __dtemp + 0 * 8 \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl 88 + 20(%%ebp) \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl __f2sng \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl __half \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fistpl 76(%%ebp) \n"

      "  fldl 88 + 28(%%ebp) \n"
      "  fldl 88 + 44(%%ebp) \n"
      "  fsubp %%st(0), %%st(1) \n"
      "  fldl __dtemp + 0 * 8 \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl 88 + 28(%%ebp) \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl __f2sng \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl __half \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fistpl 80(%%ebp) \n"
      "gfx_v_line_pos_skip1: \n"
      // end update u & v
      
      "movl 0(%%ebp), %%ebx \n"
      "movl 8(%%ebp), %%ecx \n"
      "movl 76(%%ebp), %%eax \n"
      "movl 8+88(%%ebp), %%esp \n"
      "addl %%ebx, %%ecx \n"
      "addl %%eax, %%esp \n"
      "movl %%ecx, 0(%%ebp) \n"
      "movl %%esp, 76(%%ebp) \n"

      "subl %%ebx, %%eax \n"
      "imull %%esi \n"
      "shll $16, %%edx \n"
      "shrl $16, %%eax \n"
      "addl %%eax, %%edx \n"
      "addl %%edx, %%ebx \n"
      "shrl $16, %%ebx \n"
      "andl $63, %%ebx \n"
      "movl %%ebx, __temp1 \n"

      "movl 4(%%ebp), %%ebx \n"
      "movl 12(%%ebp), %%ecx \n"
      "movl 80(%%ebp), %%eax \n"
      "movl 12+88(%%ebp), %%esp \n"
      "addl %%ebx, %%ecx \n"
      "addl %%eax, %%esp \n"
      "movl %%ecx, 4(%%ebp) \n"
      "movl %%esp, 80(%%ebp) \n"

      "subl %%ebx, %%eax \n"
      "imull %%esi \n"
      "shll $16, %%edx \n"
      "shrl $16, %%eax \n"
      "addl %%eax, %%edx \n"
      "addl %%edx, %%ebx \n"
      "andl $63 << 16, %%ebx \n"
      "shrl $16 - 6, %%ebx \n"
      "movl __temp1, %%eax \n"
      "addl %%ebx, %%eax \n"
      "leal (%%eax, %%eax, 2), %%eax \n"
      "movl __tri_data + 28, %%ebx \n"

///*
    "  movl __tri_data + 16, %%ecx \n"
    "  movb 0(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 0(%%edi) \n"
    
    "  movl __tri_data + 20, %%ecx \n"
    "  movb 1(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 1(%%edi) \n"
    
    "  movl __tri_data + 24, %%ecx \n"
    "  movb 2(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 2(%%edi) \n"
//*/
//      "movb 0(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 0(%%edi) \n"
//      "movb 1(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 1(%%edi) \n"
//      "movb 2(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 2(%%edi) \n"

      "addl $3, %%edi \n"

      "movl __temp3, %%eax \n"
      "inc %%eax \n"
      "movl %%eax, 16(%%ebp) \n"
      "movl %%eax, __temp3 \n"
      
      "subw __temp5, %%si \n"
      "jnc gfx_v_line_pos_skip2 \n"
      "  subl $88, %%ebp \n"
      "gfx_v_line_pos_skip2: \n"

      "cmp __temp4, %%eax \n"
      "jl gfx_v_line_pos_rep1 \n"
      
      "movl __esp, %%esp \n"
      "pop %%esi \n"
      "pop %%ebp \n"
    :
    :"g" (_temp7), "g" (_temp6), "g" (_bmp.texel), "g" (scrn)
    :"eax", "ebx", "ecx", "edx", "edi", "esi", "ebp", "memory"
    );

//    _texdat[DAT_MID + scan].last_ind = x + 1;
//    s -= m.valu;
//    scrn += 3;
//  }

}


///*
void gfx_v_line_neg (int y, int x1, int x2, double scan_m, int color,
  view_type view)
{
  if (x1 > x2)
    return;

  _tri_data.count = x2 - x1;
  _tri_data.back_r = _temp16;//128 << 8;
  _tri_data.back_g = _temp17;//128 << 8;
  _tri_data.back_b = _temp18;//128 << 8;
  _tri_data.bmp.texel = _bmp.texel;

  sng m = cv_sng(-scan_m);
  long s = int(65536 * (double(y + .5) - double(x1 + .5) * scan_m));
  int scrn = (unsigned int)_x2_buff + (unsigned int)3 * ((unsigned int)x1 + (unsigned int)y * (unsigned int)320);
  _temp3 = x1;
  _temp4 = x2 + 1;//int(&_texdat[DAT_MID + s_scan.whole]);
  _temp5 = m.valu;
  _temp6 = s & 65535;
  _temp7 = int(&_texdat[DAT_MID + (s >> 16)]);
  
//  while (_temp3 <= x2) {
//    int scan = s_scan.whole;//(int)floor(f_scan);

    asm (
      "push %%ebp \n"
      "push %%esi \n"
      "movl %%esp, __esp \n"
      "movl %1, %%esi \n"
      "movl %3, %%edi \n"
      "movl %0, %%ebp \n"

      // start update u & v
      "gfx_v_line_neg_rep1: \n"
      "movl 16(%%ebp), %%eax \n"
      "movl __temp3, %%edx \n"
      "cmp %%eax, %%edx \n"
      "je gfx_v_line_neg_skip1 \n"
      "  fild __temp3 \n" // load x
      "  fldl __half \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl 0 + 52(%%ebp) \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fldl 0 + 68(%%ebp) \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fstpl __dtemp + 8 * 0 \n"

      "  fldl 0 + 20(%%ebp) \n"
      "  fldl 0 + 36(%%ebp) \n"
      "  fsubp %%st(0), %%st(1) \n"
      "  fldl __dtemp + 0 * 8 \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl 0 + 20(%%ebp) \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl __f2sng \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl __half \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fistpl 0 + 0(%%ebp) \n"

      "  fldl 0 + 28(%%ebp) \n"
      "  fldl 0 + 44(%%ebp) \n"
      "  fsubp %%st(0), %%st(1) \n"
      "  fldl __dtemp + 0 * 8 \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl 0 + 28(%%ebp) \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl __f2sng \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl __half \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fistpl 0 + 4(%%ebp) \n"

      "  fild __temp3 \n" // load x
      "  fldl __half \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl 88 + 52(%%ebp) \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fldl 88 + 68(%%ebp) \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fstpl __dtemp + 8 * 0 \n"

      "  fldl 88 + 20(%%ebp) \n"
      "  fldl 88 + 36(%%ebp) \n"
      "  fsubp %%st(0), %%st(1) \n"
      "  fldl __dtemp + 0 * 8 \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl 88 + 20(%%ebp) \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl __f2sng \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl __half \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fistpl 76(%%ebp) \n"

      "  fldl 88 + 28(%%ebp) \n"
      "  fldl 88 + 44(%%ebp) \n"
      "  fsubp %%st(0), %%st(1) \n"
      "  fldl __dtemp + 0 * 8 \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl 88 + 28(%%ebp) \n"
      "  faddp %%st(0), %%st(1) \n"
      "  fldl __f2sng \n"
      "  fmulp %%st(0), %%st(1) \n"
      "  fldl __half \n"
      "  fsubrp %%st(0), %%st(1) \n"
      "  fistpl 80(%%ebp) \n"
      "gfx_v_line_neg_skip1: \n"
      // end update u & v
      
      "movl 0(%%ebp), %%ebx \n"
      "movl 8(%%ebp), %%ecx \n"
      "movl 76(%%ebp), %%eax \n"
      "movl 8+88(%%ebp), %%esp \n"
      "addl %%ebx, %%ecx \n"
      "addl %%eax, %%esp \n"
      "movl %%ecx, 0(%%ebp) \n"
      "movl %%esp, 76(%%ebp) \n"

      "subl %%ebx, %%eax \n"
      "imull %%esi \n"
      "shll $16, %%edx \n"
      "shrl $16, %%eax \n"
      "addl %%eax, %%edx \n"
      "addl %%edx, %%ebx \n"
      "shrl $16, %%ebx \n"
      "andl $63, %%ebx \n"
      "movl %%ebx, __temp1 \n"

      "movl 4(%%ebp), %%ebx \n"
      "movl 12(%%ebp), %%ecx \n"
      "movl 80(%%ebp), %%eax \n"
      "movl 12+88(%%ebp), %%esp \n"
      "addl %%ebx, %%ecx \n"
      "addl %%eax, %%esp \n"
      "movl %%ecx, 4(%%ebp) \n"
      "movl %%esp, 80(%%ebp) \n"

      "subl %%ebx, %%eax \n"
      "imull %%esi \n"
      "shll $16, %%edx \n"
      "shrl $16, %%eax \n"
      "addl %%eax, %%edx \n"
      "addl %%edx, %%ebx \n"
      "andl $63 << 16, %%ebx \n"
      "shrl $16 - 6, %%ebx \n"
      "movl __temp1, %%eax \n"
      "addl %%ebx, %%eax \n"
      "leal (%%eax, %%eax, 2), %%eax \n"
      "movl __tri_data + 28, %%ebx \n"

    "  movl __tri_data + 16, %%ecx \n"
    "  movb 0(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 0(%%edi) \n"
    
    "  movl __tri_data + 20, %%ecx \n"
    "  movb 1(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 1(%%edi) \n"
    
    "  movl __tri_data + 24, %%ecx \n"
    "  movb 2(%%ebx, %%eax), %%cl \n"
    "  movb __mult_tbl(%%ecx), %%cl \n"
    "  movb %%cl, 2(%%edi) \n"

//      "movb 0(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 0(%%edi) \n"
//      "movb 1(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 1(%%edi) \n"
//      "movb 2(%%eax, %%ebx), %%dl \n"
//      "movb %%dl, 2(%%edi) \n"

      "addl $3, %%edi \n"

      "movl __temp3, %%eax \n"
      "inc %%eax \n"
      "movl %%eax, 16(%%ebp) \n"
      "movl %%eax, __temp3 \n"
      
      "addw __temp5, %%si \n"
      "jnc gfx_v_line_neg_skip2 \n"
      "  addl $88, %%ebp \n"
      "gfx_v_line_neg_skip2: \n"

      "cmp __temp4, %%eax \n"
      "jl gfx_v_line_neg_rep1 \n"
      
      "movl __esp, %%esp \n"
      "pop %%esi \n"
      "pop %%ebp \n"
    :
    :"g" (_temp7), "g" (_temp6), "g" (_bmp.texel), "g" (scrn)
    :"eax", "ebx", "ecx", "edx", "edi", "esi", "ebp", "memory"
    );

//    _texdat[DAT_MID + scan].last_ind = x + 1;
//    s -= m.valu;
//    scrn += 3;
//  }

}
//*/

///*
void gfx_v_line_neg4 (int y, int x1, int x2, double scan_m, int color,
  view_type view)
{
  sng m = cv_sng(scan_m);
  sng s_scan = cv_sng(double(y + .5) - double(x2 + .5) * scan_m);
  
  for (int x = x2; x >= x1; x--) {
    int scan = s_scan.whole;//(int)floor(f_scan);
    if (_texdat[DAT_MID + scan].last_ind != x) {
//      update_uv(scan, x);
//      update_next_uv(scan, x);
    }
    sng ua = _texdat[DAT_MID + scan].u; //u1a + (u2a - u1a) * (double(x + .5) - _texdat[DAT_MID + scan + 0].ind1) * inv_len_a;
    sng va = _texdat[DAT_MID + scan].v;//v1a + (v2a - v1a) * (double(x + .5) - _texdat[DAT_MID + scan + 0].ind1) * inv_len_a;
    sng ub = _texdat[DAT_MID + scan].next_u;//u1b + (u2b - u1b) * (double(x + .5) - _texdat[DAT_MID + scan + 1].ind1) * inv_len_b;
    sng vb = _texdat[DAT_MID + scan].next_v;//v1b + (v2b - v1b) * (double(x + .5) - _texdat[DAT_MID + scan + 1].ind1) * inv_len_b;
    _texdat[DAT_MID + scan].u -= _texdat[DAT_MID + scan].du;
    _texdat[DAT_MID + scan].v -= _texdat[DAT_MID + scan].dv;
    _texdat[
    DAT_MID + scan].next_u -= _texdat[DAT_MID + scan + 1].du;
    _texdat[DAT_MID + scan].next_v -= _texdat[DAT_MID + scan + 1].dv;
    sng f = s_scan - cv_sng(scan);
    int u = cv_int(ua + (ub - ua) * f) & 63;
    int v = cv_int(va + (vb - va) * f) & 63;
    pxl (x, y, _bmp.texel[3 * (u + (v << 6)) + 0],
               _bmp.texel[3 * (u + (v << 6)) + 1],
               _bmp.texel[3 * (u + (v << 6)) + 2]);
    s_scan += m;
    _texdat[DAT_MID + scan].last_ind = x - 1;
  }
}


void gfx_v_line_neg5 (int y, int x1, int x2, double scan_m, int color,
  view_type view)
{
  sng m = cv_sng(scan_m);
  sng s_scan = cv_sng(double(y + .5) - double(x1 + .5) * scan_m);
  
  for (int x = x1; x <= x2; x++) {
    int scan = s_scan.whole;//(int)floor(f_scan);
    if (_texdat[DAT_MID + scan].last_ind != x) {
//      update_uv(scan, x);
//      update_next_uv(scan, x);
    }
    sng ua = _texdat[DAT_MID + scan].u; //u1a + (u2a - u1a) * (double(x + .5) - _texdat[DAT_MID + scan + 0].ind1) * inv_len_a;
    sng va = _texdat[DAT_MID + scan].v;//v1a + (v2a - v1a) * (double(x + .5) - _texdat[DAT_MID + scan + 0].ind1) * inv_len_a;
    sng ub = _texdat[DAT_MID + scan].next_u;//u1b + (u2b - u1b) * (double(x + .5) - _texdat[DAT_MID + scan + 1].ind1) * inv_len_b;
    sng vb = _texdat[DAT_MID + scan].next_v;//v1b + (v2b - v1b) * (double(x + .5) - _texdat[DAT_MID + scan + 1].ind1) * inv_len_b;
    _texdat[DAT_MID + scan].u += _texdat[DAT_MID + scan].du;
    _texdat[DAT_MID + scan].v += _texdat[DAT_MID + scan].dv;
    _texdat[DAT_MID + scan].next_u += _texdat[DAT_MID + scan + 1].du;
    _texdat[DAT_MID + scan].next_v += _texdat[DAT_MID + scan + 1].dv;
    sng f = s_scan - cv_sng(scan);
    int u = cv_int(ua + (ub - ua) * f) & 63;
    int v = cv_int(va + (vb - va) * f) & 63;
    pxl (x, y, _bmp.texel[3 * (u + (v << 6)) + 0],
               _bmp.texel[3 * (u + (v << 6)) + 1],
               _bmp.texel[3 * (u + (v << 6)) + 2]);
    s_scan -= m;
    _texdat[DAT_MID + scan].last_ind = x + 1;
  }
}

//*/

/*
void gfx_v_line2 (int x, int y1, int y2, double scan_m, int color,
  view_type view)
{
  if (y1 > y2)
    return;

/*
  if (y1 < 0)
    y1 = 0;
  else if (y1 > 199)
    y1 = 199;
    
  if (y2 < 0)
    y2 = 0;
  else if (y2 > 199)
    y2 = 199;

  sng m = cv_sng(scan_m);
  sng scan = cv_sng(y1 + .5 - scan_m * (x + .5) + DAT_MID);
  int s = cv_int(scan);
  sng f = (scan - cv_sng(s));
  double sca = (y1 + .5 - scan_m * (x + .5));
  double fra = sca - floor(y1 + .5 - scan_m * (x + .5));
  _f = (unsigned long)(fra * (double)65536);

  double etop1, ebot1, etop2, ebot2, inv_len1, inv_len2;
  int y;
  double fx = double(x) + .5;
  _dtemp[1] = double(x) + .5;
  
  if (x % 32 == 0) //{
    for (y = y1; y <= y2 + 1; y++, s++)
      update_uv(s, fx, int(&_texdat[s].u));
  else

//    if (view.z_map[x1 + y * 320].poly == _poly)
//      if (_texdat[s].last_ind != y) //{
//        update_uv(s, fy, int(&_texdat[s].u));

    for (y = y1; y <= y2 + 1; y++, s++)
      if (view.z_map[x + y * 320].poly == _poly) {
        if (_texdat[s + 0].last_ind != x) //{
          update_uv(s, fx, int(&_texdat[s + 0].u));
        if (_texdat[s + 1].last_ind != x) //{
          update_uv(s, fx, int(&_texdat[s + 1].u));
      }


s = cv_int(scan);
//update_uv(s, fy, int(&_texdat[s].u));
//_texdat[s].last_ind = y + 1;

int yc = y1 * WIDTH;
_temp3 = x + 1;

_temp4 = s * sizeof(texdat_type);
_scrn = (x + yc) * 3;
_temp2 = y2 - y1;
_temp5 = (int)_bmp.texel;
int sc = x + yc;
_temp8 = (int)(&view.z_map[sc].poly);
_temp11 = x;

//  _temp19 = (int)_shad.texel + 3 * (-0 + 128 * -0);
//  _temp19 = (int)_shad.texel + 3 * (64 - (int(_duv.x) & 63)) + 128 * (64 - (int(_duv.y) & 63));
//  _temp19 = (int)_shad.texel + 3 * ((64 - int(_duv.x)) + 128 * (64 - int(_duv.y)));
//  _temp21 = (int)_shad.texel + 3 * (64 - int(_duv.x));
//  _temp22 = 3 * (64 - int(_duv.y));
  
//  _temp21 = long(65536 * ((int)_shad.texel + 3 * (64 - _duv.x)));
//  _temp22 = long(65536 * 3 * (64 - _duv.y));
  _temp19 = (int)_shad.texel;
  _temp21 = long(65536 * (64 - _duv.x));
  _temp22 = long(65536 * (64 - _duv.y));

  if (_seg_mode == 0)
    gfx_v_seg1 (x, y1, y2, scan_m, color, view);
  else if (_seg_mode == 1)
    gfx_v_seg2 (x, y1, y2, scan_m, color, view);
  else //if (_seg_mode == 2)
    draw_solid_vert_line (y1, y2, x, color);

//  gfx_v_seg1 (x, y1, y2, scan_m, color, view);

}
*/


/*
void gfx_v_seg1b (int x, int y1, int y2, double scan_m, int color,
  view_type view)
{
/*
  if (x & 31 == 0)
  asm volatile (
    "push %%edi \n"
    "movl __x2_buff, %%edi \n"
    "addl __scrn, %%edi \n"
    "movl __f, %%esi \n"
    "push %%ebp \n"
    "movl %%esp, __temp \n"
    "leal __texdat, %%ebp \n"
    "addl __temp4, %%ebp \n"
//    "movl __poly, %%edx \n"
    "movl __temp8, %%ebx \n"

  "l_gfx_v_seg11a: \n"

    "movl (%%ebx), %%eax \n"
    "cmpl %%eax, __poly \n"
    "jne s_gfx_v_seg11a \n"


      "fldl __dtemp + 8 * 1 \n" // load x
      "fldl 0 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 0 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 0 + 20(%%ebp) \n"
      "fldl 0 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 0(%%ebp) \n"

      "fldl 0 + 28(%%ebp) \n"
      "fldl 0 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 4(%%ebp) \n"
      "movl %%ecx, 0 + 16(%%ebp) \n"

      "fldl __dtemp + 8 * 1 \n" // load x
      "fldl 88 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 88 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 88 + 20(%%ebp) \n"
      "fldl 88 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 0(%%ebp) \n"

      "fldl 88 + 28(%%ebp) \n"
      "fldl 88 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 4(%%ebp) \n"
      "movl %%ecx, 88 + 16(%%ebp) \n"

    "movl %%ebx, __temp7 \n"
    "movl 0(%%ebp), %%ecx \n"
    "movl 4(%%ebp), %%esp \n"
    "movl 8(%%ebp), %%eax \n"
    "movl 12(%%ebp), %%ebx \n"
    "addl %%ecx, %%eax \n"
    "addl %%esp, %%ebx \n"
    "movl %%eax, 0(%%ebp) \n"
    "movl %%ebx, 4(%%ebp) \n"
    "movl 88 + 0(%%ebp), %%eax \n"
    "movl 88 + 4(%%ebp), %%ebx \n"
    "subl %%ecx, %%eax \n"
    "subl %%esp, %%ebx \n"

    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%edx, %%ecx \n"

    "movl %%ebx, %%eax \n"
    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%esp, %%edx \n"

    "andl $63 << 16, %%ecx \n"
    "andl $63 << 16, %%edx \n"
    "shrl $16, %%ecx \n"
    "shrl $16 - 6, %%edx \n"
    "addl %%ecx, %%edx \n"
    "movl __temp5, %%ebx \n"
    "leal (%%edx, %%edx, 2), %%edx \n"
    "movl __temp3, %%eax \n"

    "movw 0(%%ebx, %%edx), %%cx \n"
    "movw %%cx, 0(%%edi) \n"
    "movb 2(%%ebx, %%edx), %%cl \n"
    "movb %%cl, 2(%%edi) \n"

    "movl %%eax, 16(%%ebp) \n"
    "movl __temp7, %%ebx \n"

    "s_gfx_v_seg11a: \n"

    "addl $8 * 320, %%ebx \n"
    "addl $3 * 320, %%edi \n"
    "addl $88, %%ebp \n"
  "decl __temp2 \n"
  "jge l_gfx_v_seg11a \n"

    "movl __temp, %%esp \n"
    "pop %%ebp \n"
    "pop %%edi \n"
  :
  :"eax" (_temp4), "g" (_bmp.texel)
  :"memory", "ebx", "ecx", "edx", "esi"
  );
  else

  asm volatile (
    "push %%edi \n"
    "movl __x2_buff, %%edi \n"
    "addl __scrn, %%edi \n"
    "movl __f, %%esi \n"
    "push %%ebp \n"
    "movl %%esp, __temp \n"
    "leal __texdat, %%ebp \n"
    "addl __temp4, %%ebp \n"
//    "movl __poly, %%edx \n"
    "movl __temp8, %%ebx \n"

  "l_gfx_v_seg11: \n"

//    "movl (%%ebx), %%eax \n"
//    "cmpl %%eax, __poly \n"
//    "jne s_gfx_v_seg11 \n"

    "movl 0 + 16(%%ebp), %%eax \n"
    "movl __temp11, %%ecx \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_v_seg12 \n"
      "fldl __dtemp + 8 * 1 \n" // load x
      "fldl 0 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 0 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 0 + 20(%%ebp) \n"
      "fldl 0 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 0(%%ebp) \n"

      "fldl 0 + 28(%%ebp) \n"
      "fldl 0 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 4(%%ebp) \n"
      "movl %%ecx, 0 + 16(%%ebp) \n"
    "s_gfx_v_seg12: \n"

    "movl 88 + 16(%%ebp), %%eax \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_v_seg13 \n"

      "fldl __dtemp + 8 * 1 \n" // load x
      "fldl 88 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 88 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 88 + 20(%%ebp) \n"
      "fldl 88 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 0(%%ebp) \n"

      "fldl 88 + 28(%%ebp) \n"
      "fldl 88 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 4(%%ebp) \n"
      "movl %%ecx, 88 + 16(%%ebp) \n"
    "s_gfx_v_seg13: \n"

    "movl %%ebx, __temp7 \n"
    "movl 0(%%ebp), %%ecx \n"
    "movl 4(%%ebp), %%esp \n"
    "movl 8(%%ebp), %%eax \n"
    "movl 12(%%ebp), %%ebx \n"
    "addl %%ecx, %%eax \n"
    "addl %%esp, %%ebx \n"
    "movl %%eax, 0(%%ebp) \n"
    "movl %%ebx, 4(%%ebp) \n"
    "movl 88 + 0(%%ebp), %%eax \n"
    "movl 88 + 4(%%ebp), %%ebx \n"
    "subl %%ecx, %%eax \n"
    "subl %%esp, %%ebx \n"

    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%edx, %%ecx \n"

    "movl %%ebx, %%eax \n"
    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%esp, %%edx \n"

    "andl $63 << 16, %%ecx \n"
    "andl $63 << 16, %%edx \n"
    "shrl $16, %%ecx \n"
    "shrl $16 - 6, %%edx \n"
    "addl %%ecx, %%edx \n"
    "movl __temp5, %%ebx \n"
    "leal (%%edx, %%edx, 2), %%edx \n"
    "movl __temp3, %%eax \n"

//    "xorl %%ecx, %%ecx \n"

    "movl __temp16, %%ecx \n"
    "movb 0(%%ebx, %%edx), %%cl \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 0(%%edi) \n"
    
    "movl __temp17, %%ecx \n"
    "movb 1(%%ebx, %%edx), %%cl \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 1(%%edi) \n"
    
    "movl __temp18, %%ecx \n"
    "movb 2(%%ebx, %%edx), %%cl \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 2(%%edi) \n"

    "movl %%eax, 16(%%ebp) \n"
    "movl __temp7, %%ebx \n"

    "s_gfx_v_seg11: \n"

    "addl $8 * 320, %%ebx \n"
    "addl $3 * 320, %%edi \n"
    "addl $88, %%ebp \n"
  "decl __temp2 \n"
  "jge l_gfx_v_seg11 \n"

    "movl __temp, %%esp \n"
    "pop %%ebp \n"
    "pop %%edi \n"
  :
  :"eax" (_temp4), "g" (_bmp.texel)
  :"memory", "ebx", "ecx", "edx", "esi"
  );
}


void gfx_v_seg2b (int x, int y1, int y2, double scan_m, int color,
  view_type view)
{
  asm volatile (
    "push %%edi \n"
    "movl __x2_buff, %%edi \n"
    "addl __scrn, %%edi \n"
    "movl __f, %%esi \n"
    "push %%ebp \n"
    "movl %%esp, __temp \n"
    "leal __texdat, %%ebp \n"
    "addl __temp4, %%ebp \n"
//    "movl __poly, %%edx \n"
    "movl __temp8, %%ebx \n"

  "l_gfx_v_seg21: \n"

//    "movl (%%ebx), %%eax \n"
//    "cmpl %%eax, __poly \n"
//    "jne s_gfx_v_seg21 \n"

    "movl 0 + 16(%%ebp), %%eax \n"
    "movl __temp11, %%ecx \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_vseg22 \n"
      "fldl __dtemp + 8 * 1 \n" // load y
      "fldl 0 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 0 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 0 + 20(%%ebp) \n"
      "fldl 0 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 0(%%ebp) \n"

      "fldl 0 + 28(%%ebp) \n"
      "fldl 0 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 4(%%ebp) \n"
      "movl %%ecx, 0 + 16(%%ebp) \n"
    "s_gfx_vseg22: \n"

    "movl 88 + 16(%%ebp), %%eax \n"
    "cmpl %%ecx, %%eax \n"
    "je s_gfx_vseg23 \n"

      "fldl __dtemp + 8 * 1 \n" // load y
      "fldl 88 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 88 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 88 + 20(%%ebp) \n"
      "fldl 88 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 0(%%ebp) \n"

      "fldl 88 + 28(%%ebp) \n"
      "fldl 88 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 88 + 4(%%ebp) \n"
      "movl %%ecx, 88 + 16(%%ebp) \n"
    "s_gfx_vseg23: \n"

    "movl %%ebx, __temp7 \n"
    "movl 0(%%ebp), %%ecx \n"
    "movl 4(%%ebp), %%esp \n"
    "movl 8(%%ebp), %%eax \n"
    "movl 12(%%ebp), %%ebx \n"
    "addl %%ecx, %%eax \n"
    "addl %%esp, %%ebx \n"
    "movl %%eax, 0(%%ebp) \n"
    "movl %%ebx, 4(%%ebp) \n"
    "movl 88 + 0(%%ebp), %%eax \n"
    "movl 88 + 4(%%ebp), %%ebx \n"
    "subl %%ecx, %%eax \n"
    "subl %%esp, %%ebx \n"

    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%edx, %%ecx \n" // ecx = u (16.16)

    "movl %%ebx, %%eax \n"
    "imull %%esi \n"
    "shll $16, %%edx \n"
    "shrl $16, %%eax \n"
    "addl %%eax, %%edx \n"
    "addl %%esp, %%edx \n" // edx = v (16.16)

    "movl %%ecx, %%esp \n"
    "movl %%edx, %%eax \n"
    "addl __temp21, %%esp \n"
    "addl __temp22, %%eax \n"
    "andl $127 << 16, %%esp \n"
    "andl $127 << 16, %%eax \n"
    "shrl $16, %%esp \n"
    "shrl $16 - 7, %%eax \n"
    "addl %%esp, %%eax \n"
    "leal (%%eax, %%eax, 2), %%eax \n"
    "addl __temp19, %%eax \n"

    "shrl $16, %%ecx \n"
    
//    "movl %%ecx, %%esp \n"
//    "andl $127, %%esp \n"
//    "movl %%edx, %%eax \n"
//    "andl $127 << 16, %%eax \n"
//    "shrl $16 - 7, %%eax \n"
//    "addl %%esp, %%eax \n"
//    "leal (%%eax, %%eax, 2), %%eax \n"
//    "addl __temp19, %%eax \n"

    "andl $63 << 16, %%edx \n"
    "andl $63 << 0, %%ecx \n"
    "shrl $16 - 6, %%edx \n"

    "addl %%ecx, %%edx \n"
    "movl __temp5, %%ebx \n"
    "leal (%%edx, %%edx, 2), %%edx \n"
    "movl __temp3, %%esp \n"

//    "xorl %%ecx, %%ecx \n"

//    "movl (%%eax), %%eax \n"
//    "addl __temp19, %%eax \n"
    
    "movl __temp16, %%ecx \n"
    "movl 0(%%eax), %%esp \n"
    "andl $255, %%esp \n"
    "movb 0(%%ebx, %%edx), %%cl \n"
    "shll $8, %%esp \n"
    "addl %%esp, %%ecx \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 0(%%edi) \n"
    
    "movl __temp17, %%ecx \n"
    "movl 1(%%eax), %%esp \n"
    "andl $255, %%esp \n"
    "movb 1(%%ebx, %%edx), %%cl \n"
    "shll $8, %%esp \n"
    "addl %%esp, %%ecx \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 1(%%edi) \n"
    
    "movl __temp18, %%ecx \n"
    "movl 2(%%eax), %%esp \n"
    "andl $255, %%esp \n"    
    "movb 2(%%ebx, %%edx), %%cl \n"
    "shll $8, %%esp \n"
    "addl %%esp, %%ecx \n"
    "movb __mult_tbl(%%ecx), %%cl \n"
    "movb %%cl, 2(%%edi) \n"

    "movl __temp17, %%esp \n"
    "movl %%esp, 16(%%ebp) \n"
    "movl __temp7, %%ebx \n"

    "s_gfx_v_seg21: \n"

    "addl $8 * 320, %%ebx \n"
    "addl $3 * 320, %%edi \n"
    "addl $88, %%ebp \n"
  "decl __temp2 \n"
  "jge l_gfx_v_seg21 \n"

    "movl __temp, %%esp \n"
    "pop %%ebp \n"
    "pop %%edi \n"
  :
  :"eax" (_temp4), "g" (_bmp.texel)
  :"memory", "ebx", "ecx", "edx", "esi"
  );
}
*/


double gen_poly
 (scrn_point* start, texdat_type* texdat, bmp_type bmp, point_type* ref,
  view_type view, int* rot)
{
  double scan_m;
  double scan_min, scan_max;

  for (int i = 0; i < 4; i++)
    calc_point (&ref[i], view);

/*
  if (approx_greater(ref[1].rel.z, ref[0].rel.z)) // 0, 1
    if (approx_lesser(ref[2].rel.z, ref[0].rel.z)) { // 2, 0, 1
      double dx, dy;
      calc_scan_m (&dx, &dy, ref[0], ref[1], ref[2], view);
      if (fabs(dx) >= fabs(dy))
        if (dx > 0) {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_horz_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[0], ref[2], view);

          calc_sect_tr
          (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
           scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
           *rot = 0;
        }
        else {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_horz_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[2], ref[0], view);

          calc_sect_rt
          (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
           scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
           *rot = 0;           
        }
      else
        if (dy > 0) {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_vert_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[0], ref[2], view);

          calc_sect_tr
          (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
           scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
           }
           *rot = 1;
        }
        else {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;
          
          if (!_zm) {
          init_vert_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[2], ref[0], view);

          calc_sect_rt
          (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
           scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
           *rot = 1;
        }
    }
    else if (approx_greater(ref[2].rel.z, ref[0].rel.z))
      if (approx_lesser(ref[2].rel.z, ref[1].rel.z)) { // 0, 2, 1
        double dx, dy;
        calc_scan_m (&dx, &dy, ref[0], ref[1], ref[2], view);
        if (fabs(dx) >= fabs(dy))
          if (dx > 0) {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[1], ref[0], ref[2], view);

            calc_sect_tr
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
             scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
             }
             *rot = 0;
          }
          else {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[1], ref[2], ref[0], view);

            calc_sect_rt
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
             scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
             }
             *rot = 0;
          }
        else
          if (dy > 0) {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[1], ref[0], ref[2], view);

            calc_sect_tr
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
             scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
             *rot = 1;
          }
          else {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[1], ref[2], ref[0], view);

            calc_sect_rt
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
             scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
             *rot = 1;
          }
      }
      else if (approx_greater(ref[2].rel.z, ref[1].rel.z)) { // 0, 1, 2
        double dx, dy;
        calc_scan_m (&dx, &dy, ref[1], ref[2], ref[3], view);
        if (fabs(dx) >= fabs(dy))
          if (dx > 0) {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[2], ref[1], ref[3], view);

            calc_sect_rb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
              *rot = 0;
          }
        else {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[2], ref[3], ref[1], view);

            calc_sect_br
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
              }
              *rot = 0;
        }
      else
        if (dy > 0) {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_vert_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[2], ref[1], ref[3], view);

          calc_sect_rb
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 1;
        }
        else {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_vert_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[2], ref[3], ref[1], view);

          calc_sect_br
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 1;
        }
      }
      else { // 0, 1-2
        double dx = ref[2].scr.x - ref[1].scr.x;
        double dy = ref[2].scr.y - ref[1].scr.y;
        if (fabs(dx) > fabs(dy))
          if (dx > 0) {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan_square (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[1], ref[0], ref[2], ref[3], view);

            calc_sect_tb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
          else {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan_square (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], ref[1], view);

            calc_sect_bt
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
        else
          if (dy > 0) {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan_square (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[1], ref[0], ref[2], ref[3], view);

            calc_sect_tb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
          else {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan_square (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], ref[1], view);

            calc_sect_bt
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
    }
    else { // 2-0, 1
      double dx, dy;
      calc_scan_m (&dx, &dy, ref[0], ref[1], ref[2], view);
      if (fabs(dx) >= fabs(dy))
        if (dx > 0) {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_horz_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[0], ref[2], view);

          calc_sect_tr
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 0;
        }
        else {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_horz_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[2], ref[0], view);

          calc_sect_rt
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }            
          *rot = 0;
        }
      else
        if (dy > 0) {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_vert_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[0], ref[2], view);

          calc_sect_tr
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 1;
        }
        else {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_vert_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[2], ref[0], view);

          calc_sect_rt
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 1;
        }
      }
  else if (approx_lesser(ref[1].rel.z, ref[0].rel.z)) // 1, 0
    if (approx_lesser(ref[2].rel.z, ref[1].rel.z)) { // 2, 1, 0
      double dx, dy;
      calc_scan_m (&dx, &dy, ref[0], ref[1], ref[2], view);
      if (fabs(dx) > fabs(dy))
        if (dx > 0) {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_horz_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[0], ref[3], ref[1], view);

          calc_sect_lt
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 0;
        }
        else {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_horz_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[0], ref[1], ref[3], view);

          calc_sect_tl
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 0;
        }
      else
        if (dy > 0) {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_vert_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[0], ref[3], ref[1], view);

          calc_sect_lt
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 1;
        }
        else {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          if (!_zm) {
          init_vert_scan (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[0], ref[1], ref[3], view);

          calc_sect_tl
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          }
          *rot = 1;
        }
    }
    else if (approx_greater(ref[2].rel.z, ref[1].rel.z))
      if (approx_lesser(ref[2].rel.z, ref[0].rel.z)) { // 1, 2, 0
        double dx, dy;
        calc_scan_m (&dx, &dy, ref[2], ref[3], ref[0], view);
        if (fabs(dx) > fabs(dy))
          if (dx > 0) {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], view);

            calc_sect_bl
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
          else {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[0], ref[2], view);

            calc_sect_lb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
        else
          if (dy > 0) {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], view);

            calc_sect_bl
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
          else {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[0], ref[2], view);

            calc_sect_lb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
      }
      else if (approx_greater(ref[2].rel.z, ref[0].rel.z)) { // 1, 0, 2
        double dx, dy;
        calc_scan_m (&dx, &dy, ref[2], ref[3], ref[0], view);
        if (fabs(dx) > fabs(dy))
          if (dx > 0) {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], view);

            calc_sect_bl
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
          else {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[0], ref[2], view);

            calc_sect_lb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
        else
          if (dy > 0) {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], view);

            calc_sect_bl
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
          else {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[0], ref[2], view);

            calc_sect_lb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
      }
      else { // 1, 0-2
        double dx, dy;
        calc_scan_m (&dx, &dy, ref[2], ref[3], ref[0], view);
        if (fabs(dx) > fabs(dy))
          if (dx > 0) {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], view);

            calc_sect_bl
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
          else {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[0], ref[2], view);

            calc_sect_lb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
        else
          if (dy > 0) {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], view);

            calc_sect_bl
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
          else {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[0], ref[2], view);

            calc_sect_lb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
      }
    else { // 1-2, 0
      double dx = ref[0].scr.x - ref[3].scr.x;
      double dy = ref[0].scr.y - ref[3].scr.y;
        if (fabs(dx) > fabs(dy))
          if (dx > 0) {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan_square (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], ref[1], view);

            calc_sect_bt
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
          else {
            scan_m = dy / dx;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_horz_scan_square (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[1], ref[0], ref[2], ref[3], view);

            calc_sect_tb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 0;
          }
        else
          if (dy > 0) {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan_square (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[3], ref[2], ref[0], ref[1], view);

            calc_sect_bt
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
          else {
            scan_m = dx / dy;
            double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
            line_equat edge1, edge2;

            if (!_zm) {
            init_vert_scan_square (
              scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
              &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
              ref[1], ref[0], ref[2], ref[3], view);

            calc_sect_tb
              (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
              scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
            }
            *rot = 1;
          }
    }
  else // 0-1
    if (approx_lesser(ref[2].rel.z, ref[0].rel.z)) { // 2, 0-1
      double dx = ref[1].scr.x - ref[0].scr.x;
      double dy = ref[1].scr.y - ref[0].scr.y;
      if (fabs(dx) > fabs(dy))
        if (dx > 0) {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          init_horz_scan_square (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[0], ref[3], ref[1], ref[2], view);

          calc_sect_lr
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          *rot = 0;
        }
        else {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          init_horz_scan_square (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[2], ref[1], ref[3], ref[0], view);
          calc_sect_rl
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          *rot = 0;
        }
      else
        if (dy > 0) {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          init_vert_scan_square (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[0], ref[3], ref[1], ref[2], view);
          calc_sect_lr
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          *rot = 1;
        }
        else {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          init_vert_scan_square (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[1], ref[2], ref[0], ref[3], view);
          calc_sect_rl
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          *rot = 1;
        }
    }
    else if (approx_greater(ref[2].rel.z, ref[0].rel.z)) { // 0-1, 2
      double dx = ref[3].scr.x - ref[2].scr.x;
      double dy = ref[3].scr.y - ref[2].scr.y;
      if (fabs(dx) > fabs(dy))
        if (dx > 0) {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          init_horz_scan_square (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[2], ref[1], ref[3], ref[0], view);

          calc_sect_rl
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          *rot = 0;
        }
        else {
          scan_m = dy / dx;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          init_horz_scan_square (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[0], ref[3], ref[1], ref[2], view);

          calc_sect_lr
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          *rot = 0;
        }
      else
        if (dy > 0) {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          init_vert_scan_square (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[2], ref[1], ref[3], ref[0], view);
          calc_sect_rl
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          *rot = 1;
        }
        else {
          scan_m = dx / dy;
          double n1, n2, dn1, dn2, m3d, b3d, zoom, center, p, dp;
          line_equat edge1, edge2;

          init_vert_scan_square (
            scan_m, &scan_min, &scan_max, start, &edge1, &edge2,
            &zoom, &center, &n1, &n2, &dn1, &dn2, &m3d, &b3d, &p, &dp,
            ref[0], ref[3], ref[1], ref[2], view);
          calc_sect_lr
            (texdat, bmp, edge1, edge2, &n1, &n2, dn1, dn2,
            scan_min, scan_max, scan_m, ref, zoom, center, m3d, b3d, &p, dp);
          *rot = 1;
        }
    }
    else { // 0-1-2 and 3 all equal (more or less)
      double dx = ref[1].scr.x - ref[0].scr.x;
      double dy = ref[1].scr.y - ref[0].scr.y;
      if (dx > .00001)
        if (dy > .00001)
          if (dx > dy) {
            line_equat left_edge, right_edge;
            scan_m = dy / dx;
            double y1, y2;
            double x1, x2;
            double dx1, dx2;
            double b = ref[0].scr.y - ref[0].scr.x * scan_m;
            double scan_len = ref[1].scr.x - ref[0].scr.x;
            calc_line (&left_edge, ref[0].scr, ref[3].scr);
            calc_line (&right_edge, ref[1].scr, ref[2].scr);
            find_n_and_dn (&x1, &dx1, scan_m, floor(b) - 1, left_edge.m, left_edge.b);
            find_n_and_dn (&x2, &dx2, scan_m, floor(b) - 1, right_edge.m, right_edge.b);
            calc_horz_flat_sect
             (texdat, bmp, left_edge, right_edge,
              x1, x2, dx1, dx2, 0, 0, b,
              find_horz_scan_b (ref[3].scr, scan_m), scan_m);
//            draw_poly (point, make, 4, view, 1, scan_m, 0);
              *rot = 0;
//            return;
          }
          else {
            line_equat top_edge, bot_edge;
            scan_m = dx / dy;
            double x1, x2;
            double dx1, dx2;
            double b = ref[0].scr.x - ref[0].scr.y * scan_m;
            calc_line (&top_edge, ref[0].scr, ref[3].scr);
            calc_line (&bot_edge, ref[1].scr, ref[2].scr);
            find_n_and_dn (&x1, &dx1, scan_m, ceil(b) + 1, top_edge.inv_m, top_edge.inv_b);
            find_n_and_dn (&x2, &dx2, scan_m, ceil(b) + 1, bot_edge.inv_m, bot_edge.inv_b);
            calc_u_vert_flat_sect
             (texdat, bmp, top_edge, bot_edge,
              x1, x2, dx1, dx2, 0, 0, b,
              find_vert_scan_b (ref[3].scr, scan_m), scan_m);
//            draw_poly (point, make, 4, view, 15, scan_m, 1);
            *rot = 1;
//            return;
          }
        else if (dy < -.00001)
          if (dx > -dy) {
            line_equat left_edge, right_edge;
            scan_m = dy / dx;
            double y1, y2;
            double x1, x2;
            double dx1, dx2;
            double b = ref[0].scr.y - ref[0].scr.x * scan_m;
            double scan_len = ref[1].scr.x - ref[0].scr.x;
            calc_line (&left_edge, ref[0].scr, ref[3].scr);
            calc_line (&right_edge, ref[1].scr, ref[2].scr);
            find_n_and_dn (&x1, &dx1, scan_m, floor(b) - 1, left_edge.m, left_edge.b);
            find_n_and_dn (&x2, &dx2, scan_m, floor(b) - 1, right_edge.m, right_edge.b);
            calc_horz_flat_sect
             (texdat, bmp, left_edge, right_edge,
              x1, x2, dx1, dx2, 0, 0, b,
              find_horz_scan_b (ref[3].scr, scan_m), scan_m);
//            draw_poly (point, make, 4, view, color, scan_m, 0);
            *rot = 0;
//            return;
          }
          else {
            line_equat top_edge, bot_edge;
            scan_m = dx / dy;
            double x1, x2;
            double dx1, dx2;
            double b = ref[1].scr.x - ref[1].scr.y * scan_m;
            calc_line (&top_edge, ref[0].scr, ref[3].scr);
            calc_line (&bot_edge, ref[1].scr, ref[2].scr);
            find_n_and_dn (&x1, &dx1, scan_m, floor(b) - 1, top_edge.inv_m, top_edge.inv_b);
            find_n_and_dn (&x2, &dx2, scan_m, floor(b) - 1, bot_edge.inv_m, bot_edge.inv_b);
            calc_vert_flat_sect
             (texdat, bmp, top_edge, bot_edge,
              x1, x2, dx1, dx2, 0, 0, b,
              find_vert_scan_b (ref[2].scr, scan_m), scan_m);
//            draw_poly (point, make, 4, view, 15, scan_m, 1);
            //draw_poly (point, make, 4, view, 3, scan_m, 1);
            // draw_poly (point, make, 4, view, 2, scan_m, 1);
            *rot = 1;
          }
        else {
          scan_m = 0;
          for (int s = (int)floor(ref[0].scr.y) - 1;
          s <= (int)ceil(ref[3].scr.y) + 1;
          s++) {
            texdat[s + DAT_MID].ind1 = ref[0].scr.x - .0001;
            texdat[s + DAT_MID].ind2 = ref[1].scr.x + .0001;
            texdat[s + DAT_MID].u1 = 0;
            texdat[s + DAT_MID].u2 = bmp.width;
            texdat[s + DAT_MID].v1 =
            texdat[s + DAT_MID].v2 = bmp.height *
            ((double(s) - (ref[0].scr.y))) /
              ((ref[3].scr.y) - (ref[0].scr.y));
            texdat[s + DAT_MID].inv_len = 1 / ((ref[3].scr.y) - (ref[0].scr.y));
            texdat[s + DAT_MID].du = cv_sng(
              (texdat[s + DAT_MID].u2 - texdat[s + DAT_MID].u1) * texdat[s + DAT_MID].inv_len);
            texdat[s + DAT_MID].dv = cv_sng(
              (texdat[s + DAT_MID].v2 - texdat[s + DAT_MID].v1) * texdat[s + DAT_MID].inv_len);
            texdat[s + DAT_MID].last_ind = -99;
          }
//          draw_poly (point, make, 4, view, color, 0, 0);
          *rot = 0;
//          return;
        }
      else if (dx < -.00001)
        if (dy > .00001)
          if (-dx > dy) {
            line_equat left_edge, right_edge;
            scan_m = dy / dx;
            double x1, x2;
            double dx1, dx2;
            double b = ref[1].scr.y - ref[1].scr.x * scan_m;
            calc_line (&left_edge, ref[1].scr, ref[2].scr);
            calc_line (&right_edge, ref[0].scr, ref[3].scr);
            find_n_and_dn (&x1, &dx1, scan_m, ceil(b) + 1, left_edge.m, left_edge.b);
            find_n_and_dn (&x2, &dx2, scan_m, ceil(b) + 1, right_edge.m, right_edge.b);
            calc_u_horz_flat_sect
             (texdat, bmp, left_edge, right_edge,
              x2, x1, dx2, dx1, 0, 0, b,
              find_horz_scan_b (ref[2].scr, scan_m), scan_m);
//            draw_poly (point, make, 4, view, color, scan_m, 0);
            *rot = 0;
//            return;
          }
          else {
            line_equat top_edge, bot_edge;
            scan_m = dx / dy;
            double x1, x2;
            double dx1, dx2;
            double b = ref[0].scr.x - ref[0].scr.y * scan_m;
            calc_line (&top_edge, ref[0].scr, ref[3].scr);
            calc_line (&bot_edge, ref[1].scr, ref[2].scr);
            find_n_and_dn (&x1, &dx1, scan_m, ceil(b) + 1, top_edge.inv_m, top_edge.inv_b);
            find_n_and_dn (&x2, &dx2, scan_m, ceil(b) + 1, bot_edge.inv_m, bot_edge.inv_b);
            calc_u_vert_flat_sect
             (texdat, bmp, top_edge, bot_edge,
              x1, x2, dx1, dx2, 0, 0, b,
              find_vert_scan_b (ref[3].scr, scan_m), scan_m);
//            draw_poly (point, make, 4, view, 15, scan_m, 1);
            *rot = 1;
              //draw_poly (point, make, 4, view, 3, scan_m, 1);
          }
        else if (dy < -.00001)
          if (-dx > -dy) {
            line_equat left_edge, right_edge;
            scan_m = dy / dx;
            double x1, x2;
            double dx1, dx2;
            double b = ref[1].scr.y - ref[1].scr.x * scan_m;
            calc_line (&left_edge, ref[1].scr, ref[2].scr);
            calc_line (&right_edge, ref[0].scr, ref[3].scr);
            find_n_and_dn (&x1, &dx1, scan_m, ceil(b) + 1, left_edge.m, left_edge.b);
            find_n_and_dn (&x2, &dx2, scan_m, ceil(b) + 1, right_edge.m, right_edge.b);
            calc_u_horz_flat_sect
             (texdat, bmp, left_edge, right_edge,
              x2, x1, dx2, dx1, 0, 0, b,
              find_horz_scan_b (ref[2].scr, scan_m), scan_m);
//            draw_poly (point, make, 4, view, color, scan_m, 0);
            *rot = 0;
//            return;
          }
          else {
            line_equat top_edge, bot_edge;
            scan_m = dx / dy;
            double x1, x2;
            double dx1, dx2;
            double b = ref[1].scr.x - ref[1].scr.y * scan_m;
            calc_line (&top_edge, ref[0].scr, ref[3].scr);
            calc_line (&bot_edge, ref[1].scr, ref[2].scr);
            find_n_and_dn (&x1, &dx1, scan_m, floor(b) - 1, top_edge.inv_m, top_edge.inv_b);
            find_n_and_dn (&x2, &dx2, scan_m, floor(b) - 1, bot_edge.inv_m, bot_edge.inv_b);
            calc_vert_flat_sect
             (texdat, bmp, top_edge, bot_edge,
              x1, x2, dx1, dx2, 0, 0, b,
              find_vert_scan_b (ref[2].scr, scan_m), scan_m);
//            draw_poly (point, make, 4, view, 15, scan_m, 1);
            *rot = 1;
            //draw_poly (point, make, 4, view, 3, scan_m, 1);
            // draw_poly (point, make, 4, view, 2, scan_m, 1);
//            draw_poly (point, make, 4, view, 14, scan_m, 1);
          }
        else {
          for (int s = (int)ceil(ref[1].scr.y) + 1;
          s >= (int)floor(ref[2].scr.y) - 1;
          s--) {
            texdat[(int)floor(s) + DAT_MID].ind2 = ref[2].scr.x - .0001;
            texdat[(int)floor(s) + DAT_MID].ind1 = ref[3].scr.x + .0001;
            texdat[(int)floor(s) + DAT_MID].u1 = 0;
            texdat[(int)floor(s) + DAT_MID].u2 = bmp.width - 0;
            texdat[(int)floor(s) + DAT_MID].v1 =
            texdat[(int)floor(s) + DAT_MID].v2 = bmp.height *
            ((double(s) - ref[0].scr.y) / (ref[2].scr.y - ref[1].scr.y));
            texdat[s + DAT_MID].inv_len = 1 / (ref[2].scr.y - ref[1].scr.y);
            texdat[s + DAT_MID].du = cv_sng(
              (texdat[s + DAT_MID].u2 - texdat[s + DAT_MID].u1) * texdat[s + DAT_MID].inv_len);
            texdat[s + DAT_MID].dv = cv_sng(
              (texdat[s + DAT_MID].v2 - texdat[s + DAT_MID].v1) * texdat[s + DAT_MID].inv_len);
            texdat[s + DAT_MID].last_ind = -99;
          }
//          draw_poly (point, make, 4, view, color, 0, 0);
          *rot = 0;
//          return;
        }
      else
        if (dy > .0001) {
          scan_m = 0;
          for (int s = (int)ceil(ref[0].scr.x) + 1;
          s >= (int)floor(ref[3].scr.x) - 1;
          s--) {
            double rig_x = ref[2].scr.x;
            double lef_x = ref[3].scr.x;
            texdat[(int)floor(s) + DAT_MID].ind1 = ref[0].scr.y - .0001;
            texdat[(int)floor(s) + DAT_MID].ind2 = ref[1].scr.y + .0001;
            texdat[(int)floor(s) + DAT_MID].u1 = 0;
            texdat[(int)floor(s) + DAT_MID].u2 = bmp.width;
            texdat[(int)floor(s) + DAT_MID].v1 =
            texdat[(int)floor(s) + DAT_MID].v2 = bmp.height *
            ((double(s) - ref[0].scr.x) / (ref[3].scr.x - ref[0].scr.x));
            texdat[s + DAT_MID].inv_len = 1 / (ref[3].scr.x - ref[0].scr.x);
            texdat[s + DAT_MID].du = cv_sng(
              (texdat[s + DAT_MID].u2 - texdat[s + DAT_MID].u1) * texdat[s + DAT_MID].inv_len);
            texdat[s + DAT_MID].dv = cv_sng(
              (texdat[s + DAT_MID].v2 - texdat[s + DAT_MID].v1) * texdat[s + DAT_MID].inv_len);
            texdat[s + DAT_MID].last_ind = -99;
          }
//          draw_poly (point, make, 4, view, color, 0, 1);
          *rot = 1;
        }
        else {
          scan_m = 0;
          for (int s = 0;//(int)floor(ref[1].scr.x) - 1;
          s <= 319;//(int)ceil(ref[2].scr.x) + 1;
          s++) {
            texdat[(int)floor(s) + DAT_MID].ind1 = ref[3].scr.y - .0001;
            texdat[(int)floor(s) + DAT_MID].ind2 = ref[2].scr.y + .0001;
            texdat[(int)floor(s) + DAT_MID].u1 = 0;
            texdat[(int)floor(s) + DAT_MID].u2 = bmp.width;
            texdat[(int)floor(s) + DAT_MID].v1 =
            texdat[(int)floor(s) + DAT_MID].v2 = bmp.height *
            ((double(s) - ref[1].scr.x) / (ref[2].scr.x - ref[1].scr.x));
            texdat[s + DAT_MID].inv_len = 1 / (ref[2].scr.x - ref[3].scr.x);
            texdat[s + DAT_MID].du = cv_sng(
              (texdat[s + DAT_MID].u2 - texdat[s + DAT_MID].u1) * texdat[s + DAT_MID].inv_len);
            texdat[s + DAT_MID].dv = cv_sng(
              (texdat[s + DAT_MID].v2 - texdat[s + DAT_MID].v1) * texdat[s + DAT_MID].inv_len);
            texdat[s + DAT_MID].last_ind = -99;
          }
//          draw_poly (point, make, 4, view, color, 0, 1);
          *rot = 1;
//          printf ("case B"); getch();
        }
    }
//    draw_poly (point, make, 4, view, color, scan_m);
//  printf ("(%f, %f)\n", scan_min, scan_max);
//  getch();
  
//  double* z_map;
*/
  return scan_m;
}


inline void find_x_and_dx
  (double* n, double* dn, line_equat edge, double scan_m, double scan_b)
{
  if (edge.inv_m != 0) {
    *dn = 1 / (edge.m - scan_m);
    *n = (scan_b - edge.b) * *dn;
  }
  else {
    *dn = 0;
    *n = edge.inv_b;
  }
}


inline void find_y_and_dy
  (double* n, double* dn, line_equat edge, double scan_m, double scan_b)
{
  if (edge.m != 0) {
    *dn = 1 / (edge.inv_m - scan_m);
    *n = (scan_b - edge.inv_b) * *dn;
  }
  else {
    *dn = 0;
    *n = edge.b;
  }
}


inline void find_n_and_dn
  (double* n, double* dn, double m1, double b1, double m2, double b2)
{
  *dn = 1 / (m2 - m1);
  *n = (b1 - b2) * *dn;
}


inline void find_n_and_dn2
  (double* n, double* dn, double m1, double b1, double m2, double b2)
{
  *n = (b2 * m1 + b1) / (1 - m1 * m2);
  *dn = 1 / (1 - m1 * m2);
}


inline bool approx_lesser (double a, double b)
{
  if ((b - a) > CMP_PRECISION)
    return true;
  else
    return false;
}


inline bool approx_greater (double a, double b)
{
  if ((a - b) > CMP_PRECISION)
    return true;
  else
    return false;
}


inline bool approx_equal (double a, double b)
{
  if (fabs(a - b) <= CMP_PRECISION)
    return true;
  else
    return false;
}


inline double find_vert_scan_b (point_2d point, double scan_m)
{
  return point.x - point.y * scan_m;
}


inline double find_horz_scan_b (point_2d point, double scan_m)
{
  return point.y - point.x * scan_m;
}


void calc_line (line_equat* line, scrn_point* point1, scrn_point* point2)
{
  line->m = (point2->y - point1->y) / (point2->x - point1->x);
  line->b = point1->y - point1->x * line->m;
//  line->m = cv_sng(line->real_m);

  line->inv_m = (point2->x - point1->x) / (point2->y - point1->y);
  line->inv_b = point1->x - point1->y * line->inv_m;
//  line->inv_m = cv_sng(line->real_inv_m);

//  printf ("%f, ", line->m);
/*
  if (_debug[0] == 1) {
    printf ("%f, ", line->real_b + point1->x * line->real_m);
    printf ("%f, ", point1->y);
    printf ("\n"); getch();
  }
*/
  /*
  if (fabs(line->real_m) >= 32767) {
    printf ("crap 1");
    getch();
  }
  if (fabs(line->real_inv_m) >= 32767) {
    printf ("crap 2");
    getch();
  }

  if (line->real_m == INF) {
    printf ("crap 3");
    getch();
  }
  if (line->real_inv_m == INF) {
    printf ("crap 4");
    getch();
  }
  */
}


inline void init_horz_scan (
  double scan_m, double* scan_min, double* scan_max, scrn_point* start,
  line_equat* left, line_equat* right, double* zoom, double* center,
  double* n1, double* n2, double* dn1, double* dn2, double* m3d, double* b3d,
  double* p, double* dp,
  point_type p_far, point_type p_left, point_type p_right, view_type view)
{
  calc_bmp_edge (left, p_left, p_far, view);
  calc_bmp_edge (right, p_right, p_far, view);

  *scan_min = *scan_max = find_horz_scan_b (start, scan_m);
  for (scrn_point* scr = start->next; scr != start; scr = scr->next) {
    if (find_horz_scan_b (scr, scan_m) < *scan_min)
      *scan_min = find_horz_scan_b (scr, scan_m);
    if (find_horz_scan_b (scr, scan_m) > *scan_max)
      *scan_max = find_horz_scan_b (scr, scan_m);
  }

  find_x_and_dx (n1, dn1, *left, scan_m, floor(*scan_min) - 1);
  find_x_and_dx (n2, dn2, *right, scan_m, floor(*scan_min) - 1);

  if (fabs(left->inv_m) < .0001) {
    find_n_and_dn2
      (p, dp, scan_m, floor(*scan_min) - 1, left->inv_m, left->inv_b);
    *m3d = (p_far.rel.y - p_left.rel.y) / (p_far.rel.z - p_left.rel.z);
    *b3d = p_far.rel.y - *m3d * p_far.rel.z;
    *zoom = view.zoom.y;
    *center = view.center.y;
  }
  else {
    *p = *n1;
    *dp = *dn1;
    *m3d = (p_far.rel.x - p_left.rel.x) / (p_far.rel.z - p_left.rel.z);
    *b3d = p_far.rel.x - *m3d * p_far.rel.z;
    *zoom = view.zoom.x;
    *center = view.center.x;
  }
}


inline void init_vert_scan (
  double scan_m, double* scan_min, double* scan_max, scrn_point* start,
  line_equat* left, line_equat* right, double* zoom, double* center,
  double* n1, double* n2, double* dn1, double* dn2, double* m3d, double* b3d,
  double* p, double* dp,
  point_type p_far, point_type p_left, point_type p_right, view_type view)
{
  calc_bmp_edge (left, p_left, p_far, view);
  calc_bmp_edge (right, p_right, p_far, view);

  *scan_min = *scan_max = find_vert_scan_b (start, scan_m);
  for (scrn_point* scr = start->next; scr != start; scr = scr->next) {
    if (find_vert_scan_b (scr, scan_m) < *scan_min)
      *scan_min = find_vert_scan_b (scr, scan_m);
    if (find_vert_scan_b (scr, scan_m) > *scan_max)
      *scan_max = find_vert_scan_b (scr, scan_m);
  }

  find_y_and_dy (n1, dn1, *left, scan_m, floor(*scan_min) - 1);
  find_y_and_dy (n2, dn2, *right, scan_m, floor(*scan_min) - 1);

  if (fabs(left->m) < .0001) {
    find_n_and_dn2
      (p, dp, scan_m, floor(*scan_min) - 1, left->m, left->b);
    *m3d = (p_far.rel.x - p_left.rel.x) / (p_far.rel.z - p_left.rel.z);
    *b3d = p_far.rel.x - *m3d * p_far.rel.z;
    *zoom = view.zoom.x;
    *center = view.center.x;
  }
  else {
    *p = *n1;
    *dp = *dn1;
    *m3d = (p_far.rel.y - p_left.rel.y) / (p_far.rel.z - p_left.rel.z);
    *b3d = p_far.rel.y - *m3d * p_far.rel.z;
    *zoom = view.zoom.y;
    *center = view.center.y;
  }
}


inline void init_horz_scan_square (
  double scan_m, double* scan_min, double* scan_max, scrn_point* start,
  line_equat* left, line_equat* right, double* zoom, double* center,
  double* n1, double* n2, double* dn1, double* dn2, double* m3d, double* b3d,
  double* p, double* dp,
  point_type p_far_left, point_type p_near_left,
  point_type p_far_right, point_type p_near_right, view_type view)
{
  calc_bmp_edge (left, p_near_left, p_far_left, view);
  calc_bmp_edge (right, p_near_right, p_far_right, view);

  *scan_min = *scan_max = find_horz_scan_b (start, scan_m);
  for (scrn_point* scr = start->next; scr != start; scr = scr->next) {
    if (find_horz_scan_b (scr, scan_m) < *scan_min)
      *scan_min = find_horz_scan_b (scr, scan_m);
    if (find_horz_scan_b (scr, scan_m) > *scan_max)
      *scan_max = find_horz_scan_b (scr, scan_m);
  }

  find_x_and_dx (n1, dn1, *left, scan_m, floor(*scan_min) - 1);
  find_x_and_dx (n2, dn2, *right, scan_m, floor(*scan_min) - 1);

  if (fabs(left->inv_m) < .0001) {
    find_n_and_dn2
      (p, dp, scan_m, floor(*scan_min) - 1, left->inv_m, left->inv_b);
    *m3d = (p_far_left.rel.y - p_near_left.rel.y) /
      (p_far_left.rel.z - p_near_left.rel.z);
    *b3d = p_far_left.rel.y - *m3d * p_far_left.rel.z;
    *zoom = view.zoom.y;
    *center = view.center.y;
  }
  else {
    *p = *n1;
    *dp = *dn1;
    *m3d = (p_far_left.rel.x - p_near_left.rel.x) /
      (p_far_left.rel.z - p_near_left.rel.z);
    *b3d = p_far_left.rel.x - *m3d * p_far_left.rel.z;
    *zoom = view.zoom.x;
    *center = view.center.x;
  }
}


inline void init_vert_scan_square (
  double scan_m, double* scan_min, double* scan_max, scrn_point* start,
  line_equat* top, line_equat* bot, double* zoom, double* center,
  double* n1, double* n2, double* dn1, double* dn2, double* m3d, double* b3d,
  double* p, double* dp,
  point_type p_far_top, point_type p_near_top,
  point_type p_far_bot, point_type p_near_bot, view_type view)
{
  calc_bmp_edge (top, p_near_top, p_far_top, view);
  calc_bmp_edge (bot, p_near_bot, p_far_bot, view);

  *scan_min = *scan_max = find_vert_scan_b (start, scan_m);
  for (scrn_point* scr = start->next; scr != start; scr = scr->next) {
    if (find_vert_scan_b (scr, scan_m) < *scan_min)
      *scan_min = find_vert_scan_b (scr, scan_m);
    if (find_vert_scan_b (scr, scan_m) > *scan_max)
      *scan_max = find_vert_scan_b (scr, scan_m);
  }

  find_y_and_dy (n1, dn1, *top, scan_m, floor(*scan_min) - 1);
  find_y_and_dy (n2, dn2, *bot, scan_m, floor(*scan_min) - 1);

  if (fabs(top->m) < .0001) {
    find_n_and_dn2
      (p, dp, scan_m, floor(*scan_min) - 1, top->m, top->b);
    *m3d = (p_far_top.rel.x - p_near_top.rel.x) /
      (p_far_top.rel.z - p_near_top.rel.z);
    *b3d = p_far_top.rel.x - *m3d * p_far_top.rel.z;
    *zoom = view.zoom.x;
    *center = view.center.x;
  }
  else {
    *p = *n1;
    *dp = *dn1;
    *m3d = (p_far_top.rel.y - p_near_top.rel.y) /
      (p_far_top.rel.z - p_near_top.rel.z);
    *b3d = p_far_top.rel.y - *m3d * p_far_top.rel.z;
    *zoom = view.zoom.y;
    *center = view.center.y;
  }
}


inline bool calc_scrn_edge
  (line_equat* line, point_3d p1, point_3d p2, point_2d* pa, point_2d* pb,
   view_type view)
{
  if (p1.z > p2.z)
    if (p2.z > view.z_cutoff)
      calc_2d_dydx_line
        (line, *pa = map_to_scrn (view, p1), *pb = map_to_scrn (view, p2));
    else if (p1.z > view.z_cutoff)
      calc_2d_dydx_line
        (line, *pa = map_to_scrn (view, p1), *pb = map_to_scrn (view, intrapolate_3d (p1, p2, view.z_cutoff)));
    else
      return false;
  else if (p1.z < p2.z)
    if (p1.z > view.z_cutoff)
      calc_2d_dydx_line
        (line, *pa = map_to_scrn (view, p1), *pb = map_to_scrn (view, p2));
    else if (p2.z > view.z_cutoff)
      calc_2d_dydx_line (line, *pa = map_to_scrn (view, intrapolate_3d (p1, p2, view.z_cutoff)), *pb = map_to_scrn (view, p2));
    else
      return false;
  else
    if (p1.z > view.z_cutoff)
      calc_2d_dydx_line
        (line, *pa = map_to_scrn (view, p1), *pb = map_to_scrn (view, p2));
    else
      return false;

  return true;
}


inline bool calc_scrn_edge2
  (line_equat* line, point_3d p1, point_3d p2, point_2d* pa, point_2d* pb,
   view_type view)
{
//  calc_2d_dydx_line
//    (line, map_to_scrn (view, p1), map_to_scrn (view, p2));

//  return true;
  
  if (p1.z > p2.z)
    if (p2.z > view.z_cutoff)
      calc_2d_dydx_line
        (line, *pa = map_to_scrn (view, p1), *pb = map_to_scrn (view, p2));
    else if (p1.z > view.z_cutoff)
      calc_2d_dydx_line
        (line, *pa = map_to_scrn (view, p1), *pb = map_to_scrn (view, intrapolate_3d (p1, p2, view.z_cutoff)));
  else if (p1.z < p2.z)
    if (p1.z > view.z_cutoff)
      calc_2d_dydx_line
        (line, *pa = map_to_scrn (view, p1), *pb = map_to_scrn (view, p2));
    else if (p2.z > view.z_cutoff)
      calc_2d_dydx_line (line, *pa = map_to_scrn (view, intrapolate_3d (p1, p2, view.z_cutoff)), *pb = map_to_scrn (view, p2));
    else
      return false;
  else
    if (p1.z > view.z_cutoff)
      calc_2d_dydx_line
        (line, *pa = map_to_scrn (view, p1), *pb = map_to_scrn (view, p2));
    else
      return false;

//  return true;
}


inline bool calc_horz_bmp_edge
  (line_3d l3d, line_2d* ref_2d, double scan_m, view_type view)
{
  point_2d p2d_1, p2d_2;
  point_3d p3d_1, p3d_2;

  p3d_1.x = l3d.mx * 1 + l3d.bx;
  p3d_1.y = l3d.my * 1 + l3d.by;
  p3d_1.z = 1;
  p3d_2.x = l3d.mx * 2 + l3d.bx;
  p3d_2.y = l3d.my * 2 + l3d.by;
  p3d_2.z = 2;

  p2d_1 = map_to_scrn (view, p3d_1);
  p2d_2 = map_to_scrn (view, p3d_2);

  double dx = p2d_2.x - p2d_1.x;
  double dy = p2d_2.y - p2d_1.y;

  if (is_approx_zero (dx)) {
    ref_2d->m = 0;
    ref_2d->b = p2d_1.x;
    return true;
  }
  else {
    line_equat l2d;
    l2d.m = dy / dx;
    l2d.b = p2d_1.y - p2d_1.x * l2d.m;
    ref_2d->m = 1 / (l2d.m - scan_m);
    ref_2d->b = -l2d.b * ref_2d->m;
    return false;
  }
}


inline bool calc_vert_bmp_edge
  (line_3d l3d, line_2d* ref_2d, double scan_m, view_type view)
{
  point_2d p2d_1, p2d_2;
  point_3d p3d_1, p3d_2;

  p3d_1.x = l3d.mx * 1 + l3d.bx;
  p3d_1.y = l3d.my * 1 + l3d.by;
  p3d_1.z = 1;
  p3d_2.x = l3d.mx * 2 + l3d.bx;
  p3d_2.y = l3d.my * 2 + l3d.by;
  p3d_2.z = 2;

  p2d_1 = map_to_scrn (view, p3d_1);
  p2d_2 = map_to_scrn (view, p3d_2);

  double dx = p2d_2.x - p2d_1.x;
  double dy = p2d_2.y - p2d_1.y;

  if (is_approx_zero (dy)) {
    ref_2d->m = 0;
    ref_2d->b = p2d_1.y;
    return true;
  }
  else {
    line_equat l2d;
    l2d.m = dx / dy;
    l2d.b = p2d_1.x - p2d_1.y * l2d.m;
    ref_2d->m = 1 / (l2d.m - scan_m);
    ref_2d->b = -l2d.b * ref_2d->m;
    return false;
  }
}


void calc_bmp_edge
  (line_equat* edge, point_type p_near, point_type p_far, view_type view)
{
  if (p_near.rel.z >= view.z_cutoff)
    calc_line (edge, p_near.scr, p_far.scr);
  else {
    point_type p3d;
    p3d.rel = intrapolate_3d (p_near, p_far, view.z_cutoff);
    rel_to_scr (p3d.rel, &p3d.scr, view);
    calc_line (edge, p3d.scr, p_far.scr);
  }
}


inline void calc_point (point_type* point, view_type view)
{
  // Calculates relative 3D points and 2D screen points with respect to view
  
  point->rel.x = point->abs.x - view.camera.pos.x;
  point->rel.y = point->abs.y - view.camera.pos.y;
  point->rel.z = point->abs.z - view.camera.pos.z;
  
  rotate (//point->rel.x, point->rel.z,
    &point->rel.x, &point->rel.z, 0, 0, view.camera.angle_xz);
  rotate (//point->rel.z, point->rel.y,
    &point->rel.z, &point->rel.y, 0, 0, view.camera.angle_yz);
  rotate (//point->rel.x, point->rel.y,
    &point->rel.x, &point->rel.y, 0, 0, view.camera.angle_xy);

  point->scr.x = view.center.x + (view.zoom.x * point->rel.x) / point->rel.z;
  point->scr.y = view.center.y + (view.zoom.y * point->rel.y) / point->rel.z;
}


int load_bmp (const char* filename, bmp_type* bitmap, int width, int height)
{
  int handle, len;
 
  if (_dos_open(filename, 0x02, &handle)) {
    printf ("Can't load bitmap \"%s\"'\n", filename);
    getchar();
    exit(5);
  }

  len = filelength(handle);
  bitmap->texel = (unsigned char*) malloc(len);
  if (bitmap->texel == NULL) {
    printf ("Can't init %i bytes for bitmap \"%s\"\n", len, filename);
    getchar();
    exit(5);
  }
  _dos_read(handle, bitmap->texel, len, NULL);
  _dos_close(handle);

  bitmap->width = width;
  bitmap->height = height;
  
  return 0;
}


void calc_scan_m
  (double* dx, double* dy, point_type ref1, point_type p1, point_type p2,
   view_type view)
{
  line_3d l3d;

  calc_line_3d (&l3d, p1.rel, p2.rel);
  double x2 = l3d.mx * ref1.rel.z + l3d.bx;
  double y2 = l3d.my * ref1.rel.z + l3d.by;
  *dx = view.zoom.x * (x2 - ref1.rel.x);
  *dy = view.zoom.y * (y2 - ref1.rel.y);
}


void calc_line (line_equat* line, point_2d point1, point_2d point2)
{
  line->m = (point2.y - point1.y) / (point2.x - point1.x);
  line->b = point1.y - point1.x * line->m;
  line->inv_m = (point2.x - point1.x) / (point2.y - point1.y);
  line->inv_b = point1.x - point1.y * line->inv_m;
}


inline void rotate
  (double* x, double* y, double center_x, double center_y, double angle)
{
  if (*x == center_x)
    if (*y == center_y)
      return;

  double dist = sqrt(square(*x - center_x) + square(*y - center_y));
  double a = atan2(*y - center_y, *x - center_x);

  *x = center_x + cos(angle + a) * dist;
  *y = center_y + sin(angle + a) * dist;
}


inline double square(double n)
{
  return n * n;
}


void calc_line_3d (line_3d* l3d, point_3d p1, point_3d p2)
{
  l3d->mx = (p2.x - p1.x) / (p2.z - p1.z);
  l3d->my = (p2.y - p1.y) / (p2.z - p1.z);
  l3d->bx = p1.x - l3d->mx * p1.z;
  l3d->by = p1.y - l3d->my * p1.z;
}


inline double find_vert_scan_b (scrn_point* point, double scan_m)
{
  return point->x - point->y * scan_m;
}


inline double find_horz_scan_b (scrn_point* point, double scan_m)
{
  return point->y - point->x * scan_m;
}


void calc_horz_flat_sect
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double lef_x, double rig_x, double lef_dx, double rig_dx,
   double v, double dv,
   double scan, double scan_end, double scan_m)
{
   dv = bmp.height / (scan_end - scan);
   v = dv * -(scan - floor(scan));
   
   for (int s = (int)floor(scan) - 1; s <= (int)ceil(scan_end) + 1; s++) {
     texdat[(int)floor(s) + DAT_MID].ind1 = lef_x - .000;
     texdat[(int)floor(s) + DAT_MID].ind2 = rig_x + .000;
     texdat[(int)floor(s) + DAT_MID].u1 = 0;
     texdat[(int)floor(s) + DAT_MID].u2 = bmp.width - 0;
     texdat[(int)floor(s) + DAT_MID].v1 =
     texdat[(int)floor(s) + DAT_MID].v2 = bmp.height *
       ((double(s) - (scan - 0)) / ((scan_end + 0) - (scan - 0)));
     texdat[s + DAT_MID].inv_len = 1 / (rig_x - lef_x);
     texdat[s + DAT_MID].du = cv_sng(
       (texdat[s + DAT_MID].u2 - texdat[s + DAT_MID].u1) * texdat[s + DAT_MID].inv_len);
     texdat[s + DAT_MID].dv = cv_sng(
       (texdat[s + DAT_MID].v2 - texdat[s + DAT_MID].v1) * texdat[s + DAT_MID].inv_len);
     texdat[s + DAT_MID].last_ind = -99;

//     pxl (0, scan, 1);
//     pxl ((int)floor(texdat[(int)floor(scan) + DAT_MID].u1),
//       (int)floor(texdat[(int)floor(scan) + DAT_MID].v1), 4);
//     pxl ((int)floor(texdat[(int)floor(scan) + DAT_MID].u2),
//       (int)floor(texdat[(int)floor(scan) + DAT_MID].v2), 4);
     lef_x += lef_dx;
     rig_x += rig_dx;
     v += dv;     
   }
}


void calc_vert_flat_sect
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double lef_x, double rig_x, double lef_dx, double rig_dx,
   double v, double dv,
   double scan, double scan_end, double scan_m)
{
   dv = bmp.height / (scan_end - scan);
   v = dv * -(scan - floor(scan));
   
//    printf ("%f, %f\n", scan, scan_end); getch();
    
   for (int s = (int)floor(scan) - 1; s <= (int)ceil(scan_end) + 1; s++) {
//     pxl (s + lef_x * scan_m, lef_x, 1);
//     pxl (s + rig_x * scan_m, rig_x, 1);
     texdat[(int)floor(s) + DAT_MID].ind1 = lef_x - .000;
     texdat[(int)floor(s) + DAT_MID].ind2 = rig_x + .000;
     texdat[(int)floor(s) + DAT_MID].u1 = 0;
     texdat[(int)floor(s) + DAT_MID].u2 = bmp.width;
     texdat[(int)floor(s) + DAT_MID].v1 =
     texdat[(int)floor(s) + DAT_MID].v2 = bmp.height *
       ((double(s) - scan) / (scan_end - scan));
    texdat[s + DAT_MID].inv_len = 1 / (rig_x - lef_x);
    texdat[s + DAT_MID].du = cv_sng(
      (texdat[s + DAT_MID].u2 - texdat[s + DAT_MID].u1) * texdat[s + DAT_MID].inv_len);
    texdat[s + DAT_MID].dv = cv_sng(
      (texdat[s + DAT_MID].v2 - texdat[s + DAT_MID].v1) * texdat[s + DAT_MID].inv_len);
    texdat[s + DAT_MID].last_ind = -99;

//     pxl (0, scan, 1);
//     pxl ((int)floor(texdat[(int)floor(scan) + DAT_MID].u1),
//       (int)floor(texdat[(int)floor(scan) + DAT_MID].v1), 4);
//     pxl ((int)floor(texdat[(int)floor(scan) + DAT_MID].u2),
//       (int)floor(texdat[(int)floor(scan) + DAT_MID].v2), 4);
     lef_x += lef_dx;
     rig_x += rig_dx;
     v += dv;     
   }
}


void calc_u_vert_flat_sect
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double lef_x, double rig_x, double lef_dx, double rig_dx,
   double v, double dv,
   double scan, double scan_end, double scan_m)
{
   dv = bmp.height / (scan_end - scan);
   v = dv * -(scan - floor(scan));
   
//    printf ("%f, %f\n", scan, scan_end); getch();
    
   for (int s = (int)ceil(scan) + 1; s >= (int)floor(scan_end) - 1; s--) {
//     pxl (s + lef_x * scan_m, lef_x, 1);
//     pxl (s + rig_x * scan_m, rig_x, 1);
     texdat[(int)floor(s) + DAT_MID].ind1 = lef_x - .000;
     texdat[(int)floor(s) + DAT_MID].ind2 = rig_x + .000;
     texdat[(int)floor(s) + DAT_MID].u1 = 0;
     texdat[(int)floor(s) + DAT_MID].u2 = bmp.width;
     texdat[(int)floor(s) + DAT_MID].v1 =
     texdat[(int)floor(s) + DAT_MID].v2 = bmp.height *
       ((double(s) - scan) / (scan_end - scan));
     texdat[s + DAT_MID].inv_len = 1 / (rig_x - lef_x);
     texdat[s + DAT_MID].du = cv_sng(
       (texdat[s + DAT_MID].u2 - texdat[s + DAT_MID].u1) * texdat[s + DAT_MID].inv_len);
     texdat[s + DAT_MID].dv = cv_sng(
       (texdat[s + DAT_MID].v2 - texdat[s + DAT_MID].v1) * texdat[s + DAT_MID].inv_len);
     texdat[s + DAT_MID].last_ind = -99;
     lef_x -= lef_dx;
     rig_x -= rig_dx;
     v -= dv;
   }
}


void calc_u_horz_flat_sect
  (texdat_type* texdat, bmp_type bmp, line_equat left, line_equat right,
   double lef_x, double rig_x, double lef_dx, double rig_dx,
   double v, double dv,
   double scan, double scan_end, double scan_m)
{
   dv = bmp.height / (scan_end - scan);
   v = dv * -(scan - floor(scan));
   
   for (int s = (int)ceil(scan) + 1; s >= (int)floor(scan_end) - 1; s--) {
     texdat[(int)floor(s) + DAT_MID].ind1 = lef_x - .000;
     texdat[(int)floor(s) + DAT_MID].ind2 = rig_x + .000;
     texdat[(int)floor(s) + DAT_MID].u1 = 0;
     texdat[(int)floor(s) + DAT_MID].u2 = bmp.width - 0;
     texdat[(int)floor(s) + DAT_MID].v1 =
     texdat[(int)floor(s) + DAT_MID].v2 = bmp.height *
       ((double(s) - (scan - 0 - .0)) / ((scan_end) - (scan)));
//     pxl (0, scan, 1);
//     pxl ((int)floor(texdat[(int)floor(scan) + DAT_MID].u1),
//       (int)floor(texdat[(int)floor(scan) + DAT_MID].v1), 4);
//     pxl ((int)floor(texdat[(int)floor(scan) + DAT_MID].u2),
//       (int)floor(texdat[(int)floor(scan) + DAT_MID].v2), 4);
     texdat[s + DAT_MID].inv_len = 1 / (rig_x - lef_x);
     texdat[s + DAT_MID].du = cv_sng(
       (texdat[s + DAT_MID].u2 - texdat[s + DAT_MID].u1) * texdat[s + DAT_MID].inv_len);
     texdat[s + DAT_MID].dv = cv_sng(
       (texdat[s + DAT_MID].v2 - texdat[s + DAT_MID].v1) * texdat[s + DAT_MID].inv_len);
     texdat[s + DAT_MID].last_ind = -99;
     lef_x -= lef_dx;
     rig_x -= rig_dx;
     v -= dv;
   }
}


inline void rel_to_scr (point_3d rel, scrn_point* scr, view_type view)
{
  double inv_z = 1 / rel.z;
  scr->x = view.center.x + rel.x * view.zoom.x * inv_z;
  scr->y = view.center.y + rel.y * view.zoom.y * inv_z;
}


inline void rel_to_scr (point_3d rel, point_2d* scr, view_type view)
{
  double inv_z = 1 / rel.z;
  scr->x = view.center.x + rel.x * view.zoom.x * inv_z;
  scr->y = view.center.y + rel.y * view.zoom.y * inv_z;
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


void show_bmp (bmp_type bmp, int w, int h)
{
  for (int y = 0; y < w; y++)
    for (int x = 0; x < h; x++)
      pxl (x + 0, y + 0, //128, 128, 128);
        bmp.texel[0 + 3 * (x + y * w)],
        bmp.texel[1 + 3 * (x + y * w)],
        bmp.texel[2 + 3 * (x + y * w)]);
}


inline void calc_abs_plane (plane_type* plane, point_type* point)
{
  calc_plane (plane, point[0].abs, point[1].abs, point[2].abs);
}


inline void calc_rel_plane (plane_type* plane, point_type* point)
{
  calc_plane (plane, point[0].rel, point[1].rel, point[2].rel);
}


void calc_plane
  (plane_type* plane, point_3d point1, point_3d point2, point_3d point3)
{
// plane:
//     z = m1x + b1
//     b1 = m2y + b
//     z = m1x + m2y + b

  y_line_3d line;
  point_3d point_b;
  
  if (approx_equal (point1.y, point2.y))
    if (approx_equal (point1.y, point3.y)) { // 1 = 2 = 3
      plane->b = point1.y;
      plane->y_plane = true;
      plane->m1_inf = false;
    }
    else // (1 = 2) != 3
      if (!approx_equal (point1.x, point2.x)) {
        plane->m1 = (point2.z - point1.z) / (point2.x - point1.x);
        double b1 = point3.z - point3.x * plane->m1;
        double b2 = point1.z - point1.x * plane->m1;
        plane->m2 = (b2 - b1) / (point1.y - point3.y);
        plane->b = b1 - plane->m2 * point3.y;
        plane->y_plane = false;
        plane->m1_inf = false;
      }
      else {
        plane->m2 = (point3.x - point2.x) / (point3.y - point2.y);
        plane->perp_m2 = -(point3.y - point2.y) / (point3.x - point2.x);
        plane->b = point2.x - point2.y * plane->m2;
        plane->y_plane = false;
        plane->m1_inf = true;
      }
  else
    if (approx_equal (point1.y, point3.y)) { // (3 = 1) != 2
      calc_y_line_3d (&line, point2, point3);
      point_b.x = line.mx * point1.y + line.bx;
      point_b.z = line.mz * point1.y + line.bz;
      if (!approx_equal(point_b.x, point1.x)) {
//        _test[0] = 331;
//        plane->m1 = (point_b.z - point1.z) / (point_b.x - point1.x);
        plane->m1 = (point1.z - point3.z) / (point1.x - point3.x);
        double b1 = point2.z - point2.x * plane->m1;
        double b2 = point3.z - point3.x * plane->m1;
        plane->m2 = (b2 - b1) / (point3.y - point2.y);
        plane->b = b1 - plane->m2 * point2.y;
        plane->y_plane = false;
        plane->m1_inf = false;
      }
      else { // possible bugs here
        line_3d z_line;
        if (!approx_equal (point1.z, point2.z)) {
//          _test[0] = 332;
          calc_line_3d (&z_line, point1, point2);
          double x = z_line.mx * point3.z + z_line.bx;
          double y = z_line.my * point3.z + z_line.by;
          plane->m2 = (point3.x - x) / (point3.y - y);
          plane->perp_m2 = -(point3.y - y) / (point3.x - x);
          plane->b = x - y * plane->m2;
        }
        else {
//          _test[0] = 333;
          calc_line_3d (&z_line, point2, point3);
          double x = z_line.mx * point1.z + z_line.bx;
          double y = z_line.my * point1.z + z_line.by;
          plane->m2 = (point1.x - x) / (point1.y - y);
          plane->perp_m2 = -(point1.y - y) / (point1.x - x);
          plane->b = x - y * plane->m2;
        }
        plane->m1_inf = true;
        plane->y_plane = false;
      }
    }
    else {
      calc_y_line_3d (&line, point1, point2);
      point_b.x = line.mx * point3.y + line.bx;
      point_b.z = line.mz * point3.y + line.bz;
      if (!approx_equal (point_b.x, point3.x)) {
        plane->m1 = (point_b.z - point3.z) / (point_b.x - point3.x);
        double b1 = point3.z - point3.x * plane->m1;
        double b2 = point1.z - point1.x * plane->m1;
        plane->m2 = (b2 - b1) / (point1.y - point3.y);
        plane->b = b1 - plane->m2 * point3.y;
        plane->y_plane = false;
        plane->m1_inf = false;
      }
      else {
        line_3d z_line;
        if (!approx_equal (point1.z, point2.z)) {
          calc_line_3d (&z_line, point1, point2);
          double x = z_line.mx * point3.z + z_line.bx;
          double y = z_line.my * point3.z + z_line.by;
          plane->m2 = (point3.x - x) / (point3.y - y);
          plane->b = x - y * plane->m2;
        }
        else {
          calc_line_3d (&z_line, point2, point3);
          double x = z_line.mx * point1.z + z_line.bx;
          double y = z_line.my * point1.z + z_line.by;
          plane->m2 = (point1.x - x) / (point1.y - y);
          plane->b = x - y * plane->m2;
        }
        plane->m1_inf = true;
        plane->y_plane = false;
      }
    }

  plane->sec_m1 = 1 / cos(atan(plane->m1));
  plane->perp_m1 = -1 / plane->m1;
  plane->perp_m2 = -1 / plane->m2;

//*/
/*
  if (point1.y != point2.y) {
    if (point1.y == point3.y)
      // (1 = 3) != 2
//    }
    // 1 != 2 != 3
    calc_y_line_3d (&line, point1, point2);
    point_b.x = line.mx * point3.y + line.bx;
    point_b.z = line.mz * point3.y + line.bz;
    if (point_b.x != point3.x) {
      plane->m1 = (point_b.z - point3.z) / (point_b.x - point3.x);
      double b1 = point3.z - point3.x * plane->m1;
      double b2 = point1.z - point1.x * plane->m1;
      plane->m2 = (b2 - b1) / (point1.y - point3.y);
      plane->b = b1 - plane->m2 * point3.y;
      plane->y_plane = false;
      plane->m1_inf = false;
    }
    else {
      line_3d z_line;
      if (point1.z != point2.z) {
        calc_line_3d (&z_line, point1, point2);
        double x = z_line.mx * point3.z + z_line.bx;
        double y = z_line.my * point3.z + z_line.by;
        plane->m2 = (point3.x - x) / (point3.y - y);
        plane->b = x - y * plane->m2;
      }
      else {
        calc_line_3d (&z_line, point2, point3);
        double x = z_line.mx * point1.z + z_line.bx;
        double y = z_line.my * point1.z + z_line.by;
        plane->m2 = (point1.x - x) / (point1.y - y);
        plane->b = x - y * plane->m2;
      }
      plane->m1_inf = true;
      plane->y_plane = false;
    }
  }
  else
    if (point1.y != point3.y) // 3 != (1 = 2)
      if (point1.x != point2.x) {
        plane->m1 = (point2.z - point1.z) / (point2.x - point1.x);
        double b1 = point3.z - point3.x * plane->m1;
        double b2 = point1.z - point1.x * plane->m1;
        plane->m2 = (b2 - b1) / (point1.y - point3.y);
        plane->b = b1 - plane->m2 * point3.y;
        plane->y_plane = false;
        plane->m1_inf = false;
      }
      else {
        plane->m2 = (point3.x - point2.x) / (point3.y - point2.y);
        plane->b = point2.x - point2.y * plane->m2;
        plane->y_plane = false;
        plane->m1_inf = true;
      }
    else { // 1 = 2 = 3
      plane->b = point1.y;
      plane->y_plane = true;
      plane->m1_inf = false;
    }
*/
}


int point_relative_to_plane (plane_type plane, point_3d point)
{
  if (plane.y_plane)
    if (point.y < plane.b)
      return -1;
    else if (point.y > plane.b)
      return 1;
    else
      return 0;
  else {
    if (plane.m1_inf) {
      double x = plane.m2 * point.y + plane.b;
      if (point.x < x)
        return -1;
      else if (point.x > x)
        return 1;
      else
        return 0;
    }
    else {
      double z = plane.m1 * point.x + plane.m2 * point.y + plane.b;
      if (point.z < z)
        return -1;
      else if (point.z > z)
        return 1;
      else
        return 0;
    }
  }
}


int point_rel_to_plane (plane_type plane, point_3d point, double rad)
{
  if (plane.y_plane) {
    _dtemp[25] = 1;
    if (point.y < plane.b + rad) {
      return -1;
    }
    else if (point.y > plane.b + rad) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    if (plane.m1_inf) {
      _dtemp[25] = 5;
      double x = plane.m2 * point.y + plane.b;
      if (point.x < x)
        return -1;
      else if (point.x > x)
        return 1;
      else
        return 0;
    }
    else {
      double z = plane.m1 * point.x + plane.m2 * point.y + plane.b;
      _dtemp[25] = 6;
      if (point.z < z)
        return -1;
      else if (point.z > z)
        return 1;
      else
        return 0;
    }
  }
}


inline void calc_y_line_3d (y_line_3d* line, point_3d p1, point_3d p2)
{
//  x = m1y + b1
//  z = m2y + b2

  line->mx = (p2.x - p1.x) / (p2.y - p1.y);
  line->bx = p1.x - line->mx * p1.y;
  line->mz = (p2.z - p1.z) / (p2.y - p1.y);
  line->bz = p1.z - line->mz * p1.y;
}


inline y_line_3d calc_y_line_3d (point_3d p1, point_3d p2)
{
//  x = m1y + b1
//  z = m2y + b2
  y_line_3d line;
  
  line.mx = (p2.x - p1.x) / (p2.y - p1.y);
  line.bx = p1.x - line.mx * p1.y;
  line.mz = (p2.z - p1.z) / (p2.y - p1.y);
  line.bz = p1.z - line.mz * p1.y;

  return line;
}


point_3d intersect_line_plane_3d (plane_type plane, line_3d line)
{
// struct line_3d {
//  double mx, my, bx, by;
// };

//struct plane_type {
//  double m1, m2, b;
//  bool y_plane;
//  bool m1_inf;

  point_3d p;

  p.z = (line.bx * plane.m1 + line.by * plane.m2 + plane.b) /
        (1 - plane.m1 * line.mx - plane.m2 * line.my);
  p.x = line.mx * p.z + line.bx;
  p.y = line.my * p.z + line.by;
  
  return p;
}


inline line_3d calc_aim_line (entity_type ent)
{
// struct entity_type {
//  point_3d pos;
//  double angle_xy;
//  double angle_xz;
//  double angle_yz;
  line_3d line;

  line.mx = tan(ent.angle_xz);
  line.my = tan(ent.angle_yz);
  line.bx = ent.pos.x - line.mx * ent.pos.z;
  line.by = ent.pos.y - line.my * ent.pos.z;
  
  return line;
}


void gprint (char* s, double v, int n)
{
  char str[1000];
  sprintf (str, s, v);
  printg (str, n);
}


void printg (char* str, int yc)
{
  for (int i = 0; str[i] != '\0'; i++) {
    int c = (8 * 8 * 3) * ((int)str[i] - 32);
    int pos = i * 8;
    for (int y = 0; y < 8; y++)
      for (int x = 0; x < 8; x++)
        pxl (x + pos, y + yc * 8,
          _text[c + 0 + x * 3 + y * 24],
          _text[c + 1 + x * 3 + y * 24],
          _text[c + 2 + x * 3 + y * 24]
        );
  }
}


unsigned char* load_text()
{
  int handle, len;
  unsigned char* p;
  
  if (_dos_open("text.dat", 0x02, &handle)) {
    printf ("Can't load text");
    getchar();
    exit(5);
  }

  len = filelength(handle);
  p = (unsigned char*) malloc(len);
  if (p == NULL) {
    printf ("crap");
    getchar();
    exit(5);
  }
  _dos_read(handle, p, len, NULL);
  _dos_close(handle);

  return p;
}


inline bool is_real_num (double n)
{
  if (n == _inf_double)
    return false;
  else if (n == -_inf_double)
    return false;
  else if (n == _nan_double)
    return false;
  else
    return true;
}


inline dbl_pair calc_illum_area
  (point_3d ref_orig, point_3d ref_u, point_3d ref_v, double z,
   double u_len, double v_len, double rad,
   tri_cut* tc, bmp_type bmp, view_type view)
{
  line_3d u_axis, v_axis;
  calc_line_3d (&u_axis, ref_orig, ref_u);
  calc_line_3d (&v_axis, ref_orig, ref_v);
  dbl_pair uv;
  point_3d corner[4];
  point_3d center;

  center.x = 0;
  center.y = 0;
  center.z = z;
  
  if (approx_equal (ref_orig.z, ref_u.z))
    if (approx_equal (ref_orig.z, ref_v.z))
      if (approx_equal (ref_orig.x, ref_u.x)) {
        uv.x = bmp.width * ((center.y - ref_orig.y) / (ref_u.y - ref_orig.y));
        uv.y = bmp.height * ((center.x - ref_orig.x) / (ref_v.x - ref_orig.x));
        corner[0].x = center.x - rad;
        corner[0].y = center.y - rad;
        corner[1].x = center.x + rad;
        corner[1].y = center.y - rad;
        corner[2].x = center.x + rad;
        corner[2].y = center.y + rad;
        corner[3].x = center.x - rad;
        corner[3].y = center.y + rad;
        corner[0].z = corner[1].z = corner[2].z = corner[3].z = z;
      }
      else if (approx_equal (ref_orig.x, ref_v.x)) {
        uv.x = bmp.width * ((center.x - ref_orig.x) / (ref_u.x - ref_orig.x));
        uv.y = bmp.height * ((center.y - ref_orig.y) / (ref_v.y - ref_orig.y));
        corner[0].x = center.x - rad;
        corner[0].y = center.y - rad;
        corner[1].x = center.x + rad;
        corner[1].y = center.y - rad;
        corner[2].x = center.x + rad;
        corner[2].y = center.y + rad;
        corner[3].x = center.x - rad;
        corner[3].y = center.y + rad;
        corner[0].z = corner[1].z = corner[2].z = corner[3].z = z;        
      }
      else {
        line_equat l0u = calc_3d_dydx_line (ref_orig, ref_u);
        line_equat l0v = calc_3d_dydx_line (ref_orig, ref_v);

        double bcu = center.y - l0v.m * center.x;
        double bcv = center.y - l0u.m * center.x;
        double bu = ref_u.y - l0v.m * ref_u.x;
        double bv = ref_v.y - l0u.m * ref_v.x;
        point_3d ua, ub, va, vb, int_u, int_v;
        ua.z = ub.z = va.z = vb.z = center.z;
        double uf = (bcu - l0v.b) / (bu - l0v.b);
        double vf = (bcv - l0u.b) / (bv - l0u.b);
        int_u.x = ref_orig.x + (ref_u.x - ref_orig.x) * uf;
        int_u.y = ref_orig.y + (ref_u.y - ref_orig.y) * uf;
        int_v.x = ref_orig.x + (ref_v.x - ref_orig.x) * vf;
        int_v.y = ref_orig.y + (ref_v.y - ref_orig.y) * vf;
        int_u.z = int_v.z = center.z;

        uv.x = bmp.width * ((int_u.x - ref_orig.x) / (ref_u.x - ref_orig.x));
        uv.y = bmp.height * ((int_v.x - ref_orig.x) / (ref_v.x - ref_orig.x));
        
        double dxu = rad * ((ref_u.x - ref_orig.x) / u_len);
        double dyu = rad * ((ref_u.y - ref_orig.y) / u_len);
        double dxv = rad * ((ref_v.x - ref_orig.x) / v_len);
        double dyv = rad * ((ref_v.y - ref_orig.y) / v_len);

        double xua = int_u.x + dxu;
        double yua = int_u.y + dyu;
        double xub = int_u.x - dxu;
        double yub = int_u.y - dyu;
        double xva = int_v.x + dxv;
        double yva = int_v.y + dyv;
        double xvb = int_v.x - dxv;
        double yvb = int_v.y - dyv;

        double bua = yua - l0v.m * xua;
        double bub = yub - l0v.m * xub;
        double bva = yva - l0u.m * xva;
        double bvb = yvb - l0u.m * xvb;

        corner[0].z = corner[1].z = corner[2].z = corner[3].z = z;
        corner[0].x = (bvb - bub) / (l0v.m - l0u.m);
        corner[1].x = (bvb - bua) / (l0v.m - l0u.m);
        corner[2].x = (bva - bua) / (l0v.m - l0u.m);
        corner[3].x = (bva - bub) / (l0v.m - l0u.m);
        corner[0].y = corner[0].x * l0v.m + bub;
        corner[1].y = corner[1].x * l0v.m + bua;
        corner[2].y = corner[2].x * l0u.m + bva;
        corner[3].y = corner[3].x * l0u.m + bva;
      }
    else {
      line_3d v_axis;
      calc_line_3d (&v_axis, ref_orig, ref_v);
      point_3d int_u, int_v;
      int_v.x = v_axis.mx * center.z + v_axis.bx;
      int_v.y = v_axis.my * center.z + v_axis.by;
      int_v.z = center.z;
      double ubx = center.x - center.z * v_axis.mx;
      double uby = center.y - center.z * v_axis.my;
      int_u.x = ref_orig.z * v_axis.mx + ubx;
      int_u.y = ref_orig.z * v_axis.my + uby;
      int_u.z = ref_orig.z;
      if (approx_equal (ref_u.x, ref_orig.x)) {
        uv.x = bmp.width * ((int_u.y - ref_orig.y) / (ref_u.y - ref_orig.y));
        uv.y = bmp.height * ((int_v.z - ref_orig.z) / (ref_v.z - ref_orig.z));
      }
      else {
        uv.x = bmp.width * ((int_u.x - ref_orig.x) / (ref_u.x - ref_orig.x));
        uv.y = bmp.height * ((int_v.z - ref_orig.z) / (ref_v.z - ref_orig.z));
      }
      double dvz = rad * ((ref_orig.z - ref_v.z) / v_len);
      double vza = int_v.z + dvz;
      double vzb = int_v.z - dvz;
      double dux = rad * ((ref_u.x - ref_orig.x) / u_len);
      double uxa = int_u.x + dux;
      double uxb = int_u.x - dux;
      double duy = rad * ((ref_u.y - ref_orig.y) / u_len);
      double uya = int_u.y + duy;
      double uyb = int_u.y - duy;
      double xuba = uxa - v_axis.mx * ref_orig.z;
      double yuba = uya - v_axis.my * ref_orig.z;
      double xubb = uxb - v_axis.mx * ref_orig.z;
      double yubb = uyb - v_axis.my * ref_orig.z;
      corner[0].x = v_axis.mx * vza + xubb;
      corner[0].y = v_axis.my * vza + yubb;
      corner[0].z = vza;
      corner[1].x = v_axis.mx * vza + xuba;
      corner[1].y = v_axis.my * vza + yuba;
      corner[1].z = vza;
      corner[2].x = v_axis.mx * vzb + xuba;
      corner[2].y = v_axis.my * vzb + yuba;
      corner[2].z = vzb;
      corner[3].x = v_axis.mx * vzb + xubb;
      corner[3].y = v_axis.my * vzb + yubb;
      corner[3].z = vzb;
    }
  else
    if (approx_equal (ref_orig.z, ref_v.z)) {
      line_3d u_axis;
      calc_line_3d (&u_axis, ref_orig, ref_u);
//      _test[0] = 999;
      point_3d int_u, int_v;
      int_u.z = center.z;
      int_u.x = u_axis.mx * center.z + u_axis.bx;
      int_u.y = u_axis.my * center.z + u_axis.by;
      dbl_pair b_v;
      b_v.x = center.x - u_axis.mx * center.z;
      b_v.y = center.y - u_axis.my * center.z;
      int_v.z = ref_orig.z;
      int_v.x = u_axis.mx * ref_orig.z + b_v.x;
      int_v.y = u_axis.my * ref_orig.z + b_v.y;

      uv.x = bmp.width * ((int_u.z - ref_orig.z) / (ref_u.z - ref_orig.z));
      if (approx_equal (ref_v.x, ref_orig.x))
        uv.y = bmp.height * ((int_v.y - ref_orig.y) / (ref_v.y - ref_orig.y));
      else
        uv.y = bmp.height * ((int_v.x - ref_orig.x) / (ref_v.x - ref_orig.x));
        
      double dxv = rad * ((ref_orig.x - ref_v.x) / v_len);
      double dyv = rad * ((ref_orig.y - ref_v.y) / v_len);
      double dzu = rad * ((ref_orig.z - ref_u.z) / u_len);
      double zua = center.z + dzu;
      double zub = center.z - dzu;
      dbl_pair b_va, b_vb;
      b_va.x = b_v.x + dxv;
      b_va.y = b_v.y + dyv;
      b_vb.x = b_v.x - dxv;
      b_vb.y = b_v.y - dyv;
      corner[0].x = u_axis.mx * zua + b_vb.x;
      corner[0].y = u_axis.my * zua + b_vb.y;
      corner[0].z = zua;
      corner[1].x = u_axis.mx * zua + b_va.x;
      corner[1].y = u_axis.my * zua + b_va.y;
      corner[1].z = zua;
      corner[2].x = u_axis.mx * zub + b_va.x;
      corner[2].y = u_axis.my * zub + b_va.y;
      corner[2].z = zub;
      corner[3].x = u_axis.mx * zub + b_vb.x;
      corner[3].y = u_axis.my * zub + b_vb.y;
      corner[3].z = zub;
    }
    else
      if (approx_equal (u_axis.mx, v_axis.mx)) {
//        _test[0] = 1;
        double inv_uv = 1 / (v_axis.my - u_axis.my);
        double zu = (v_axis.my * z + u_axis.by) * inv_uv;
        double zv = -(u_axis.my * z + v_axis.by) * inv_uv;
        uv.x = ((zu - ref_orig.z) / (ref_u.z - ref_orig.z)) * bmp.width;
        uv.y = ((zv - ref_orig.z) / (ref_v.z - ref_orig.z)) * bmp.height;

        double dzu = ((ref_orig.z - ref_u.z) * rad) / u_len;
        double dzv = ((ref_orig.z - ref_v.z) * rad) / v_len;
        point_3d ua, ub, va, vb;
  
        ua.z = zu - dzu;
        ua.x = u_axis.mx * ua.z + u_axis.bx;
        ua.y = u_axis.my * ua.z + u_axis.by;
  
        ub.z = zu + dzu;
        ub.x = u_axis.mx * ub.z + u_axis.bx;
        ub.y = u_axis.my * ub.z + u_axis.by;

        va.z = zv - dzv;
        va.x = v_axis.mx * va.z + v_axis.bx;
        va.y = v_axis.my * va.z + v_axis.by;
  
        vb.z = zv + dzv;
        vb.x = v_axis.mx * vb.z + v_axis.bx;
        vb.y = v_axis.my * vb.z + v_axis.by;

        double bua_xz = ua.x - v_axis.mx * ua.z;
        double bub_xz = ub.x - v_axis.mx * ub.z;
        double bva_xz = va.x - u_axis.mx * va.z;
        double bvb_xz = vb.x - u_axis.mx * vb.z;

        double bua_yz = ua.y - v_axis.my * ua.z;
        double bub_yz = ub.y - v_axis.my * ub.z;
        double bva_yz = va.y - u_axis.my * va.z;
        double bvb_yz = vb.y - u_axis.my * vb.z;

        double inv_uvm = 1 / (u_axis.my - v_axis.my);
        corner[0].z = (bub_yz - bvb_yz) * inv_uvm;
        corner[0].x = u_axis.mx * corner[0].z + bvb_xz;
        corner[0].y = u_axis.my * corner[0].z + bvb_yz;
        corner[1].z = (bua_yz - bvb_yz) * inv_uvm; // (u_axis.mx - v_axis.mx);
        corner[1].x = u_axis.mx * corner[1].z + bvb_xz;
        corner[1].y = u_axis.my * corner[1].z + bvb_yz;
        corner[2].z = (bua_yz - bva_yz) * inv_uvm; // (u_axis.mx - v_axis.mx);
        corner[2].x = u_axis.mx * corner[2].z + bva_xz;
        corner[2].y = u_axis.my * corner[2].z + bva_yz;
        corner[3].z = (bub_yz - bva_yz) * inv_uvm; // (u_axis.mx - v_axis.mx);
        corner[3].x = u_axis.mx * corner[3].z + bva_xz;
        corner[3].y = u_axis.my * corner[3].z + bva_yz;
      }
      else {
//        _test[0] = 2;
        double inv_uv = 1 / (v_axis.mx - u_axis.mx);
        double zu = (v_axis.mx * z + u_axis.bx) * inv_uv;
        double zv = -(u_axis.mx * z + v_axis.bx) * inv_uv;
        uv.x = ((zu - ref_orig.z) / (ref_u.z - ref_orig.z)) * bmp.width;
        uv.y = ((zv - ref_orig.z) / (ref_v.z - ref_orig.z)) * bmp.height;

        double dzu = ((ref_orig.z - ref_u.z) * rad) / u_len;
        double dzv = ((ref_orig.z - ref_v.z) * rad) / v_len;
        point_3d ua, ub, va, vb;

        ua.z = zu - dzu;
        ua.x = u_axis.mx * ua.z + u_axis.bx;
        ua.y = u_axis.my * ua.z + u_axis.by;
  
        ub.z = zu + dzu;
        ub.x = u_axis.mx * ub.z + u_axis.bx;
        ub.y = u_axis.my * ub.z + u_axis.by;

        va.z = zv - dzv;
        va.x = v_axis.mx * va.z + v_axis.bx;
        va.y = v_axis.my * va.z + v_axis.by;
  
        vb.z = zv + dzv;
        vb.x = v_axis.mx * vb.z + v_axis.bx;
        vb.y = v_axis.my * vb.z + v_axis.by;

        double bua_xz = ua.x - v_axis.mx * ua.z;
        double bub_xz = ub.x - v_axis.mx * ub.z;
        double bva_xz = va.x - u_axis.mx * va.z;
        double bvb_xz = vb.x - u_axis.mx * vb.z;

        double bua_yz = ua.y - v_axis.my * ua.z;
        double bub_yz = ub.y - v_axis.my * ub.z;
        double bva_yz = va.y - u_axis.my * va.z;
        double bvb_yz = vb.y - u_axis.my * vb.z;

        double inv_uvm = 1 / (u_axis.mx - v_axis.mx);
        corner[0].z = (bub_xz - bvb_xz) * inv_uvm;
        corner[0].x = u_axis.mx * corner[0].z + bvb_xz;
        corner[0].y = u_axis.my * corner[0].z + bvb_yz;
        corner[1].z = (bua_xz - bvb_xz) * inv_uvm; // (u_axis.mx - v_axis.mx);
        corner[1].x = u_axis.mx * corner[1].z + bvb_xz;
        corner[1].y = u_axis.my * corner[1].z + bvb_yz;
        corner[2].z = (bua_xz - bva_xz) * inv_uvm; // (u_axis.mx - v_axis.mx);
        corner[2].x = u_axis.mx * corner[2].z + bva_xz;
        corner[2].y = u_axis.my * corner[2].z + bva_yz;
        corner[3].z = (bub_xz - bva_xz) * inv_uvm; // (u_axis.mx - v_axis.mx);
        corner[3].x = u_axis.mx * corner[3].z + bva_xz;
        corner[3].y = u_axis.my * corner[3].z + bva_yz;
      }

  tc->edge_exist[0] =
  calc_scrn_edge
    (&tc->line[0], corner[0], corner[1], &tc->qa[0], &tc->qb[0], view);
  tc->edge_exist[1] =
  calc_scrn_edge
    (&tc->line[1], corner[1], corner[2], &tc->qa[1], &tc->qb[1], view);
  tc->edge_exist[2] =
  calc_scrn_edge
    (&tc->line[2], corner[2], corner[3], &tc->qa[2], &tc->qb[2], view);
  tc->edge_exist[3] =
  calc_scrn_edge
    (&tc->line[3], corner[3], corner[0], &tc->qa[3], &tc->qb[3], view);

  return uv;
}


inline void calc_illum_area3
  (point_3d ref_orig, point_3d ref_u, point_3d ref_v,
   double u_len, double v_len, double rad, double int_zu, double int_zv,
   tri_cut* tc, view_type view)
{
  point_3d corner[4];
  double dzu = ((ref_orig.z - ref_u.z) * rad) / u_len;
  double dzv = ((ref_orig.z - ref_v.z) * rad) / v_len;
  line_3d u_axis, v_axis;
  point_3d ua, ub, va, vb;
  
  calc_line_3d (&u_axis, ref_orig, ref_u);
  calc_line_3d (&v_axis, ref_orig, ref_v);
  
  ua.z = int_zu - dzu;
  ua.x = u_axis.mx * ua.z + u_axis.bx;
  ua.y = u_axis.my * ua.z + u_axis.by;
  
  ub.z = int_zu + dzu;
  ub.x = u_axis.mx * ub.z + u_axis.bx;
  ub.y = u_axis.my * ub.z + u_axis.by;

  va.z = int_zv - dzv;
  va.x = v_axis.mx * va.z + v_axis.bx;
  va.y = v_axis.my * va.z + v_axis.by;
  
  vb.z = int_zv + dzv;
  vb.x = v_axis.mx * vb.z + v_axis.bx;
  vb.y = v_axis.my * vb.z + v_axis.by;

  double bua_xz = ua.x - v_axis.mx * ua.z;
  double bub_xz = ub.x - v_axis.mx * ub.z;
  double bva_xz = va.x - u_axis.mx * va.z;
  double bvb_xz = vb.x - u_axis.mx * vb.z;

  double bua_yz = ua.y - v_axis.my * ua.z;
  double bub_yz = ub.y - v_axis.my * ub.z;
  double bva_yz = va.y - u_axis.my * va.z;
  double bvb_yz = vb.y - u_axis.my * vb.z;

  double inv_uvm = 1 / (u_axis.mx - v_axis.mx);
  corner[0].z = (bub_xz - bvb_xz) * inv_uvm;
  corner[0].x = u_axis.mx * corner[0].z + bvb_xz;
  corner[0].y = u_axis.my * corner[0].z + bvb_yz;
  corner[1].z = (bua_xz - bvb_xz) * inv_uvm; // (u_axis.mx - v_axis.mx);
  corner[1].x = u_axis.mx * corner[1].z + bvb_xz;
  corner[1].y = u_axis.my * corner[1].z + bvb_yz;
  corner[2].z = (bua_xz - bva_xz) * inv_uvm; // (u_axis.mx - v_axis.mx);
  corner[2].x = u_axis.mx * corner[2].z + bva_xz;
  corner[2].y = u_axis.my * corner[2].z + bva_yz;
  corner[3].z = (bub_xz - bva_xz) * inv_uvm; // (u_axis.mx - v_axis.mx);
  corner[3].x = u_axis.mx * corner[3].z + bva_xz;
  corner[3].y = u_axis.my * corner[3].z + bva_yz;

  tc->edge_exist[0] =
  calc_scrn_edge
    (&tc->line[0], corner[0], corner[1], &tc->qa[0], &tc->qb[0], view);
  tc->edge_exist[1] =
  calc_scrn_edge
    (&tc->line[1], corner[1], corner[2], &tc->qa[1], &tc->qb[1], view);
  tc->edge_exist[2] =
  calc_scrn_edge
    (&tc->line[2], corner[2], corner[3], &tc->qa[2], &tc->qb[2], view);
  tc->edge_exist[3] =
  calc_scrn_edge
    (&tc->line[3], corner[3], corner[0], &tc->qa[3], &tc->qb[3], view);
}


inline dbl_pair calc_illum_area2
  (point_3d ref_orig, point_3d ref_u, point_3d ref_v,
   double u_len, double v_len, double rad, double z, bmp_type bmp,
   tri_cut* tc, view_type view)
{
  double int_zu, int_zv;
  line_3d u_axis, v_axis;
  calc_line_3d (&u_axis, ref_orig, ref_u);
  calc_line_3d (&v_axis, ref_orig, ref_v);
  double zu = (0 - v_axis.mx * z - u_axis.bx) / (u_axis.mx - v_axis.mx);
  double zv = (0 - u_axis.mx * z - v_axis.bx) / (v_axis.mx - u_axis.mx);
  dbl_pair uv;
  point_3d u, v;
  
  u.z = zu;
//  u->x = u_axis.mx * zu + u_axis.bx;
//  u->y = u_axis.my * zu + u_axis.by;
  
  v.z = zv;
//  v->x = v_axis.mx * zv + v_axis.bx;
//  v->y = v_axis.my * zv + v_axis.by;

  uv.x = ((zu - ref_orig.z) / (ref_u.z - ref_orig.z)) * bmp.width;
  uv.y = ((zv - ref_orig.z) / (ref_v.z - ref_orig.z)) * bmp.height;
  int_zu = uv.x; int_zv = uv.y;

  point_3d corner[4];
  double dzu = ((ref_orig.z - ref_u.z) * rad) / u_len;
  double dzv = ((ref_orig.z - ref_v.z) * rad) / v_len;
//  line_3d u_axis, v_axis;
  point_3d ua, ub, va, vb;
  
//  calc_line_3d (&u_axis, ref_orig, ref_u);
//  calc_line_3d (&v_axis, ref_orig, ref_v);
  
  ua.z = int_zu - dzu;
  ua.x = u_axis.mx * ua.z + u_axis.bx;
  ua.y = u_axis.my * ua.z + u_axis.by;
  
  ub.z = int_zu + dzu;
  ub.x = u_axis.mx * ub.z + u_axis.bx;
  ub.y = u_axis.my * ub.z + u_axis.by;

  va.z = int_zv - dzv;
  va.x = v_axis.mx * va.z + v_axis.bx;
  va.y = v_axis.my * va.z + v_axis.by;
  
  vb.z = int_zv + dzv;
  vb.x = v_axis.mx * vb.z + v_axis.bx;
  vb.y = v_axis.my * vb.z + v_axis.by;

  double bua_xz = ua.x - v_axis.mx * ua.z;
  double bub_xz = ub.x - v_axis.mx * ub.z;
  double bva_xz = va.x - u_axis.mx * va.z;
  double bvb_xz = vb.x - u_axis.mx * vb.z;

  double bua_yz = ua.y - v_axis.my * ua.z;
  double bub_yz = ub.y - v_axis.my * ub.z;
  double bva_yz = va.y - u_axis.my * va.z;
  double bvb_yz = vb.y - u_axis.my * vb.z;

  corner[0].z = (bub_xz - bvb_xz) / (u_axis.mx - v_axis.mx);
  corner[0].x = u_axis.mx * corner[0].z + bvb_xz;
  corner[0].y = u_axis.my * corner[0].z + bvb_yz;
  corner[1].z = (bua_xz - bvb_xz) / (u_axis.mx - v_axis.mx);
  corner[1].x = u_axis.mx * corner[1].z + bvb_xz;
  corner[1].y = u_axis.my * corner[1].z + bvb_yz;
  corner[2].z = (bua_xz - bva_xz) / (u_axis.mx - v_axis.mx);
  corner[2].x = u_axis.mx * corner[2].z + bva_xz;
  corner[2].y = u_axis.my * corner[2].z + bva_yz;
  corner[3].z = (bub_xz - bva_xz) / (u_axis.mx - v_axis.mx);
  corner[3].x = u_axis.mx * corner[3].z + bva_xz;
  corner[3].y = u_axis.my * corner[3].z + bva_yz;

  tc->edge_exist[0] =
  calc_scrn_edge
    (&tc->line[0], corner[0], corner[1], &tc->qa[0], &tc->qb[0], view);
  tc->edge_exist[1] =
  calc_scrn_edge
    (&tc->line[1], corner[1], corner[2], &tc->qa[1], &tc->qb[1], view);
  tc->edge_exist[2] =
  calc_scrn_edge
    (&tc->line[2], corner[2], corner[3], &tc->qa[2], &tc->qb[2], view);
  tc->edge_exist[3] =
  calc_scrn_edge
    (&tc->line[3], corner[3], corner[0], &tc->qa[3], &tc->qb[3], view);

  return uv;    
}


inline void calc_illum_area
  (tri_cut* tc, plane_type pl, view_type view)
{
  point_3d p3d[4];

  p3d[0] = calc_light_corner (pl, -20, -20);
  p3d[1] = calc_light_corner (pl, 20, -20);
  p3d[2] = calc_light_corner (pl, 20, 20);
  p3d[3] = calc_light_corner (pl, -20, 20);

  tc->edge_exist[0] =
  calc_scrn_edge
    (&tc->line[0], p3d[0], p3d[1], &tc->qa[0], &tc->qb[0], view);
  tc->edge_exist[1] =
  calc_scrn_edge
    (&tc->line[1], p3d[1], p3d[2], &tc->qa[1], &tc->qb[1], view);
  tc->edge_exist[2] =
  calc_scrn_edge
    (&tc->line[2], p3d[2], p3d[3], &tc->qa[2], &tc->qb[2], view);
  tc->edge_exist[3] =
  calc_scrn_edge
    (&tc->line[3], p3d[3], p3d[0], &tc->qa[3], &tc->qb[3], view);
}


inline dbl_pair calc_uv_with_z
  (double* zu, double* zv, point_3d ref_orig, point_3d ref_u, point_3d ref_v,
   double z, bmp_type bmp)
{
  line_3d u_axis, v_axis;
  calc_line_3d (&u_axis, ref_orig, ref_u);
  calc_line_3d (&v_axis, ref_orig, ref_v);
  dbl_pair uv;
  
  double inv_uv = 1 / (v_axis.mx - u_axis.mx);
  *zu = (v_axis.mx * z + u_axis.bx) * inv_uv;
  *zv = -(u_axis.mx * z + v_axis.bx) * inv_uv;// (v_axis.mx - u_axis.mx);
  uv.x = ((*zu - ref_orig.z) / (ref_u.z - ref_orig.z)) * bmp.width;
  uv.y = ((*zv - ref_orig.z) / (ref_v.z - ref_orig.z)) * bmp.height;

  return uv;
}


void create_quad (
  point_type* point, point_type* ref,
  double center_x, double center_y, double z,
  double angle_xz, double angle_yz, double angle_xy,
  double obj_width, double obj_height,
  double ref_x, double ref_y,
  int txl_width, int txl_height)
{
  point[0].abs.x = (center_x - obj_width * .5) * UNIT_PER_FOOT;
  point[0].abs.y = (center_y - obj_height * .5) * UNIT_PER_FOOT;
  point[0].abs.z = z * UNIT_PER_FOOT;
  point[1].abs.x = (center_x + obj_width * .5) * UNIT_PER_FOOT;
  point[1].abs.y = (center_y - obj_height * .5) * UNIT_PER_FOOT;
  point[1].abs.z = z * UNIT_PER_FOOT;
  point[2].abs.x = (center_x + obj_width * .5) * UNIT_PER_FOOT;
  point[2].abs.y = (center_y + obj_height * .5) * UNIT_PER_FOOT;
  point[2].abs.z = z * UNIT_PER_FOOT;
  point[3].abs.x = (center_x - obj_width * .5) * UNIT_PER_FOOT;
  point[3].abs.y = (center_y + obj_height * .5) * UNIT_PER_FOOT;
  point[3].abs.z = z * UNIT_PER_FOOT;

  double ref_w = double(txl_width) * FOOT_PER_TEXEL;
  double ref_h = double(txl_height) * FOOT_PER_TEXEL;
  
  ref[0].abs.x = ref_x * UNIT_PER_FOOT;
  ref[0].abs.y = ref_y * UNIT_PER_FOOT;
  ref[0].abs.z = z * UNIT_PER_FOOT;
  ref[1].abs.x = (ref_x + ref_w) * UNIT_PER_FOOT;
  ref[1].abs.y = ref_y * UNIT_PER_FOOT;
  ref[1].abs.z = z * UNIT_PER_FOOT;
  ref[2].abs.x = (ref_x + ref_w) * UNIT_PER_FOOT;
  ref[2].abs.y = (ref_y + ref_h) * UNIT_PER_FOOT;
  ref[2].abs.z = z * UNIT_PER_FOOT;
  ref[3].abs.x = ref_x * UNIT_PER_FOOT;
  ref[3].abs.y = (ref_y + ref_h) * UNIT_PER_FOOT;
  ref[3].abs.z = z * UNIT_PER_FOOT;
  
//  point_3d center;
//  center.x = (obj_x - obj_width * .5) * UNIT_PER_FOOT;
//  center.y = (obj_y - obj_height * .5) * UNIT_PER_FOOT;
  
  int i;
  for (i = 0; i < 4; i++) {
    rotate (&point[i].abs.x, &point[i].abs.z, center_x * UNIT_PER_FOOT, z * UNIT_PER_FOOT, angle_xz);
    rotate (&point[i].abs.z, &point[i].abs.y, z * UNIT_PER_FOOT, center_y * UNIT_PER_FOOT, angle_yz);
    rotate (&ref[i].abs.x, &ref[i].abs.z, center_x * UNIT_PER_FOOT, z * UNIT_PER_FOOT, angle_xz);
    rotate (&ref[i].abs.z, &ref[i].abs.y, z * UNIT_PER_FOOT, center_y * UNIT_PER_FOOT, angle_yz);
  }
//  gprint ("<<%f>>", ref[i].abs.x);
}


void init_tex_plane
  (point_3d ref_0, point_3d ref_u, point_3d ref_v, poly_dat* dat,
   int* rot, bmp_type* bmp, view_type view)
{
/*
struct uv_calc_ref {
  line_2d edge1, edge2;
  line_2d ref_2d;
  line_3d ref_3d;
  double zoom;
  double center;
};
*/
  uv_calc_ref uv_init;
  int dydx = 0, dxdy = 1;
  uv_init.orig = ref_0.z;
  uv_init.bmp = bmp;
  
  if (approx_equal (ref_0.z, ref_u.z))
    if (approx_equal (ref_0.z, ref_v.z))
      if (fabs(ref_0.x - ref_u.x) < fabs(ref_0.y - ref_u.y)) { // flat
        _test[0] = 1;
        dat->rot = 1;
        point_2d p0 = map_to_scrn (view, ref_0);
        point_2d pu = map_to_scrn (view, ref_u);
        point_2d pv = map_to_scrn (view, ref_v);
        dat->scan_m = (pu.x - p0.x) / (pu.y - p0.y);
        uv_init.s1 = p0.x - p0.y * dat->scan_m;
        uv_init.s2 = pv.x - pv.y * dat->scan_m;
        line_equat top;
        calc_line (&top, p0, pv);
        double bot_b = pu.y - top.m * pu.x;
        uv_init.edge1.m = uv_init.edge2.m = top.m / (1 - dat->scan_m * top.m);
        uv_init.edge1.b = top.b / (1 - dat->scan_m * top.m);
        uv_init.edge2.b = bot_b / (1 - dat->scan_m * top.m);
        uv_init.jump = 1;
        uv_init.inv_diff_1 = 1 / (uv_init.s2 - uv_init.s1);
      }
      else { // flat
        _test[0] = 2;
        dat->rot = 1;
        point_2d p0 = map_to_scrn (view, ref_0);
        point_2d pu = map_to_scrn (view, ref_u);
        point_2d pv = map_to_scrn (view, ref_v);
        dat->scan_m = (pv.x - p0.x) / (pv.y - p0.y);
        uv_init.s1 = p0.x - p0.y * dat->scan_m;
        uv_init.s2 = pu.x - pu.y * dat->scan_m;
        line_equat top;
        calc_line (&top, p0, pu);
        double bot_b = pv.y - top.m * pv.x;
        uv_init.edge1.m =
        uv_init.edge2.m = top.m / (1 - dat->scan_m * top.m);
        uv_init.edge1.b = top.b / (1 - dat->scan_m * top.m);
        uv_init.edge2.b = bot_b / (1 - dat->scan_m * top.m);
        uv_init.jump = 2;
        uv_init.inv_diff_1 = 1 / (uv_init.s2 - uv_init.s1);
      }
    else { // u-0, v
      double dx = (ref_u.x - ref_0.x) * view.zoom.x;
      double dy = (ref_u.y - ref_0.y) * view.zoom.y;
      uv_init.inv_diff_1 = 1 / (ref_v.z - ref_0.z);
      uv_init.jump = 3;
      if (fabs(dx) > fabs(dy)) {
        dat->scan_m = dy / dx;
        dat->rot = dydx;
        line_3d l3d_a = calc_line_3d (ref_0, ref_v);
        line_3d l3d_b = calc_parll_line_3d (l3d_a, ref_u);
        calc_horz_bmp_edge (l3d_b, &uv_init.edge2, dat->scan_m, view);
        if (calc_horz_bmp_edge (l3d_a, &uv_init.edge1, dat->scan_m, view)) {
          _test[0] = 3;
          uv_init.zoom = view.zoom.y;
          uv_init.center = view.center.y;
          uv_init.ref_2d.m = 1;
          uv_init.ref_2d.b = dat->scan_m * uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.my;
          uv_init.ref_3d.b = l3d_a.by;
        }
        else {
          _test[0] = 4;
          uv_init.zoom = view.zoom.x;
          uv_init.center = view.center.x;
          uv_init.ref_2d.m = uv_init.edge1.m;
          uv_init.ref_2d.b = uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.mx;
          uv_init.ref_3d.b = l3d_a.bx;
        }
      }
      else {
        dat->scan_m = dx / dy;
        dat->rot = dxdy;
        line_3d l3d_a = calc_line_3d (ref_0, ref_v);
        line_3d l3d_b = calc_parll_line_3d (l3d_a, ref_u);
        calc_vert_bmp_edge (l3d_b, &uv_init.edge2, dat->scan_m, view);
        if (calc_vert_bmp_edge (l3d_a, &uv_init.edge1, dat->scan_m, view)) {
          _test[0] = 5;
          uv_init.zoom = view.zoom.x;
          uv_init.center = view.center.x;
          uv_init.ref_2d.m = 1;
          uv_init.ref_2d.b = dat->scan_m * uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.mx;
          uv_init.ref_3d.b = l3d_a.bx;
        }
        else {
          _test[0] = 6;
          uv_init.zoom = view.zoom.y;
          uv_init.center = view.center.y;
          uv_init.ref_2d.m = uv_init.edge1.m;
          uv_init.ref_2d.b = uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.my;
          uv_init.ref_3d.b = l3d_a.by;
        }
      }
    }
  else
    if (approx_equal (ref_0.z, ref_v.z)) {
      double dx = (ref_v.x - ref_0.x) * view.zoom.x;
      double dy = (ref_v.y - ref_0.y) * view.zoom.y;
      uv_init.jump = 4;
      uv_init.inv_diff_1 = 1 / (ref_u.z - ref_0.z);
      if (fabs(dx) > fabs(dy)) {
        dat->scan_m = dy / dx;
        dat->rot = dydx;
        line_3d l3d_a = calc_line_3d (ref_0, ref_u);
        line_3d l3d_b = calc_parll_line_3d (l3d_a, ref_v);
        calc_horz_bmp_edge (l3d_b, &uv_init.edge2, dat->scan_m, view);
        if (calc_horz_bmp_edge (l3d_a, &uv_init.edge1, dat->scan_m, view)) {
          _test[0] = 7;
          uv_init.zoom = view.zoom.y;
          uv_init.center = view.center.y;
          uv_init.ref_2d.m = 1;
          uv_init.ref_2d.b = dat->scan_m * uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.my;
          uv_init.ref_3d.b = l3d_a.by;
        }
        else {
          _test[0] = 8;
          uv_init.zoom = view.zoom.x;
          uv_init.center = view.center.x;
          uv_init.ref_2d.m = uv_init.edge1.m;
          uv_init.ref_2d.b = uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.mx;
          uv_init.ref_3d.b = l3d_a.bx;
        }
      }
      else {
        dat->scan_m = dx / dy;
        dat->rot = dxdy;
        line_3d l3d_a = calc_line_3d (ref_0, ref_u);
        line_3d l3d_b = calc_parll_line_3d (l3d_a, ref_v);
        calc_vert_bmp_edge (l3d_b, &uv_init.edge2, dat->scan_m, view);
        if (calc_vert_bmp_edge (l3d_a, &uv_init.edge1, dat->scan_m, view)) {
          _test[0] = 9;
          uv_init.zoom = view.zoom.x;
          uv_init.center = view.center.x;
          uv_init.ref_2d.m = 1;
          uv_init.ref_2d.b = dat->scan_m * uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.mx;
          uv_init.ref_3d.b = l3d_a.bx;
        }
        else {
          _test[0] = 10;
          uv_init.zoom = view.zoom.y;
          uv_init.center = view.center.y;
          uv_init.ref_2d.m = uv_init.edge1.m;
          uv_init.ref_2d.b = uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.my;
          uv_init.ref_3d.b = l3d_a.by;
        }
      }
    }
    else {
      line_3d l3d_a = calc_line_3d (ref_0, ref_u);
      line_3d l3d_b = calc_line_3d (ref_0, ref_v);
      double x2 = l3d_a.mx * ref_v.z + l3d_a.bx;
      double y2 = l3d_a.my * ref_v.z + l3d_a.by;
      double dx = view.zoom.x * (x2 - ref_v.x);
      double dy = view.zoom.y * (y2 - ref_v.y);
      uv_init.inv_diff_1 = 1 / (ref_u.z - ref_0.z);
      uv_init.inv_diff_2 = 1 / (ref_v.z - ref_0.z);
      uv_init.jump = 5;
      
      if (fabs(dx) > fabs(dy)) {
        dat->scan_m = dy / dx;
        dat->rot = dydx;
        calc_horz_bmp_edge (l3d_b, &uv_init.edge2, dat->scan_m, view);
        if (calc_horz_bmp_edge (l3d_a, &uv_init.edge1, dat->scan_m, view)) {
          _test[0] = 11;
          uv_init.zoom = view.zoom.y;
          uv_init.center = view.center.y;
          uv_init.ref_2d.m = 1;
          uv_init.ref_2d.b = dat->scan_m * uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.my;
          uv_init.ref_3d.b = l3d_a.by;
        }
        else {
          _test[0] = 12;
          uv_init.zoom = view.zoom.x;
          uv_init.center = view.center.x;
          uv_init.ref_2d.m = uv_init.edge1.m;
          uv_init.ref_2d.b = uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.mx;
          uv_init.ref_3d.b = l3d_a.bx;
        }
      }
      else {
        dat->scan_m = dx / dy;
        dat->rot = dxdy;
        calc_vert_bmp_edge (l3d_b, &uv_init.edge2, dat->scan_m, view);
        if (calc_vert_bmp_edge (l3d_a, &uv_init.edge1, dat->scan_m, view)) {
          _test[0] = 13;
          uv_init.zoom = view.zoom.x;
          uv_init.center = view.center.x;
          uv_init.ref_2d.m = 1;
          uv_init.ref_2d.b = dat->scan_m * uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.mx;
          uv_init.ref_3d.b = l3d_a.bx;
        }
        else {
          _test[0] = 14;
          uv_init.zoom = view.zoom.y;
          uv_init.center = view.center.y;
          uv_init.ref_2d.m = uv_init.edge1.m;
          uv_init.ref_2d.b = uv_init.edge1.b;
          uv_init.ref_3d.m = l3d_a.my;
          uv_init.ref_3d.b = l3d_a.by;
        }
      }
    }
/***************************************************************************/
/*
    asm (
//      "movl %0, %%eax \n"
//      "movl %1, %%ebx \n"
      "movl %2, %%ecx \n"
      "call _update_uv__Fv"
    :
    :"g"(&uv_init), "g"(&_texdat[s + DAT_MID]), "g"(s)
    :"ecx"
    );
*/
  _uv_init = uv_init;

//  for (int s = -320 - 200; s <= 320 + 200; s++)
//    if (!_texdat[s + DAT_MID].init)
//      update_uv (&uv_init, &_texdat[s + DAT_MID], s);
}


void update_uv ()
  //eax = uv_calc_ref* init, ecx = texdat_type* dat, edx = int s
{
  uv_calc_ref* init;
  texdat_type* dat;
  int s;
  
  asm (
    "movl %%eax, %0 \n"
    "movl %%ebx, %1 \n"
    "movl %%ecx, %2 \n"
  :"=g" (init), "=g" (dat), "=g" (s)
  :
  :"memory"
  );
  
  init = &_uv_init;
  dat = &_texdat[s + DAT_MID];
  
  if (init->jump == 1) {
    dat->ind1 = init->edge1.m * s + init->edge1.b;
    dat->ind2 = init->edge2.m * s + init->edge2.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    dat->u1 = 0;
    dat->u2 = init->bmp->width;
    dat->v1 =
    dat->v2 = init->bmp->height * (s - init->s1) * init->inv_diff_1;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }
  else if (init->jump == 2) {
    dat->ind1 = init->edge1.m * s + init->edge1.b;
    dat->ind2 = init->edge2.m * s + init->edge2.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    dat->u1 =
    dat->u2 = init->bmp->width * (s - init->s1) * init->inv_diff_1;
    dat->v1 = 0;
    dat->v2 = init->bmp->height;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }
  else if (init->jump == 3) {
    dat->ind1 = init->edge1.m * s + init->edge1.b;
    dat->ind2 = init->edge2.m * s + init->edge2.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    double p = init->ref_2d.m * s + init->ref_2d.b;
    double z = (init->zoom * init->ref_3d.b) /
      (p - init->center - init->zoom * init->ref_3d.m);
    dat->u1 = 0;
    dat->u2 = init->bmp->width;
    dat->v1 =
    dat->v2 = (z - init->orig) * init->inv_diff_1 *  init->bmp->height;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }
  else if (init->jump == 4) {
    dat->ind1 = init->edge1.m * s + init->edge1.b;
    dat->ind2 = init->edge2.m * s + init->edge2.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    double p = init->ref_2d.m * s + init->ref_2d.b;
    double z = (init->zoom * init->ref_3d.b) /
      (p - init->center - init->zoom * init->ref_3d.m);
    dat->u1 =
    dat->u2 = (z - init->orig) * init->inv_diff_1 * init->bmp->width;
    dat->v1 = 0;
    dat->v2 = init->bmp->height;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }
  else if (init->jump == 5) {
    dat->ind1 = init->edge2.m * s + init->edge2.b;
    dat->ind2 = init->edge1.m * s + init->edge1.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    double p = init->ref_2d.m * s + init->ref_2d.b;
    double z = (init->zoom * init->ref_3d.b) /
      (p - init->center - init->zoom * init->ref_3d.m);
    dat->u1 = 0;
    dat->u2 = (z - init->orig) * init->inv_diff_1 * init->bmp->width;
    dat->v1 = (z - init->orig) * init->inv_diff_2 * init->bmp->height;
    dat->v2 = 0;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }

  if (s < _dtemp[20])
    _dtemp[20] = s;
  if (s > _dtemp[21])
    _dtemp[21] = s;

  _dtemp[22]++;
}


void update_uv (uv_calc_ref* init, texdat_type* dat, int s)
{
  if (init->jump == 1) {
    dat->ind1 = init->edge1.m * s + init->edge1.b;
    dat->ind2 = init->edge2.m * s + init->edge2.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    dat->u1 = 0;
    dat->u2 = init->bmp->width;
    dat->v1 =
    dat->v2 = init->bmp->height * (s - init->s1) * init->inv_diff_1;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }
  else if (init->jump == 2) {
    dat->ind1 = init->edge1.m * s + init->edge1.b;
    dat->ind2 = init->edge2.m * s + init->edge2.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    dat->u1 =
    dat->u2 = init->bmp->width * (s - init->s1) * init->inv_diff_1;
    dat->v1 = 0;
    dat->v2 = init->bmp->height;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }
  else if (init->jump == 3) {
    dat->ind1 = init->edge1.m * s + init->edge1.b;
    dat->ind2 = init->edge2.m * s + init->edge2.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    double p = init->ref_2d.m * s + init->ref_2d.b;
    double z = (init->zoom * init->ref_3d.b) /
      (p - init->center - init->zoom * init->ref_3d.m);
    dat->u1 = 0;
    dat->u2 = init->bmp->width;
    dat->v1 =
    dat->v2 = (z - init->orig) * init->inv_diff_1 *  init->bmp->height;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }
  else if (init->jump == 4) {
    dat->ind1 = init->edge1.m * s + init->edge1.b;
    dat->ind2 = init->edge2.m * s + init->edge2.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    double p = init->ref_2d.m * s + init->ref_2d.b;
    double z = (init->zoom * init->ref_3d.b) /
      (p - init->center - init->zoom * init->ref_3d.m);
    dat->u1 =
    dat->u2 = (z - init->orig) * init->inv_diff_1 * init->bmp->width;
    dat->v1 = 0;
    dat->v2 = init->bmp->height;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }
  else if (init->jump == 5) {
    dat->ind1 = init->edge2.m * s + init->edge2.b;
    dat->ind2 = init->edge1.m * s + init->edge1.b;
    dat->inv_len = 1 / (dat->ind2 - dat->ind1);
    dat->last_ind = -99;
    double p = init->ref_2d.m * s + init->ref_2d.b;
    double z = (init->zoom * init->ref_3d.b) /
      (p - init->center - init->zoom * init->ref_3d.m);
    dat->u1 = 0;
    dat->u2 = (z - init->orig) * init->inv_diff_1 * init->bmp->width;
    dat->v1 = (z - init->orig) * init->inv_diff_2 * init->bmp->height;
    dat->v2 = 0;
    dat->du = cv_sng((dat->u2 - dat->u1) * dat->inv_len);
    dat->dv = cv_sng((dat->v2 - dat->v1) * dat->inv_len);
  }

  if (s < _dtemp[20])
    _dtemp[20] = s;
  if (s > _dtemp[21])
    _dtemp[21] = s;

  _dtemp[22]++;

  dat->init = 1;
}


inline line_equat calc_3d_dydx_line (point_3d p1, point_3d p2)
{
  line_equat l;

  l.m = (p2.y - p1.y) / (p2.x - p1.x);
  l.b = p1.y - l.m * p1.x;
  
  return l;
}


void test_funct ()
{
//  int temp;
  
//  asm (
//    "movl %%ecx, %0 \n"
//  :"=g" (temp)
//  :
//  :"memory"
//  );

  _dtemp[20]++;
}


/*
void update_uv (int scan, int x)
{
//  _test[1]++;
/*
  double inv_len = _texdat[DAT_MID + scan].inv_len;
  double u1 = _texdat[DAT_MID + scan].u1;
  double v1 = _texdat[DAT_MID + scan].v1;
  double u2 = _texdat[DAT_MID + scan].u2;
  double v2 = _texdat[DAT_MID + scan].v2;
  double u = u1 + (u2 - u1) * (double(x + .5) - _texdat[DAT_MID + scan].ind1) * inv_len;
  double v = v1 + (v2 - v1) * (double(x + .5) - _texdat[DAT_MID + scan].ind1) * inv_len;
  _texdat[DAT_MID + scan].u = cv_sng(u);
  _texdat[DAT_MID + scan].v = cv_sng(v);
  _temp1 = x;

  asm volatile (
      "push %%ebp \n"
      "movl %0, %%ebp \n"

      "fild __temp1 \n" // load x
      "fldl __half \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl 0 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 0 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 0 + 20(%%ebp) \n"
      "fldl 0 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 0(%%ebp) \n"

      "fldl 0 + 28(%%ebp) \n"
      "fldl 0 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 0 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 0 + 4(%%ebp) \n"

      "pop %%ebp \n"
    :
    :"g" (&_texdat[DAT_MID + scan])
    :"memory"
  );
}


void update_next_uv (int scan, int x)
{
/*
  double inv_len = _texdat[DAT_MID + scan + 1].inv_len;
  double u1 = _texdat[DAT_MID + scan + 1].u1;
  double v1 = _texdat[DAT_MID + scan + 1].v1;
  double u2 = _texdat[DAT_MID + scan + 1].u2;
  double v2 = _texdat[DAT_MID + scan + 1].v2;
  double u = u1 + (u2 - u1) * (double(x + .5) - _texdat[DAT_MID + scan + 1].ind1) * inv_len;
  double v = v1 + (v2 - v1) * (double(x + .5) - _texdat[DAT_MID + scan + 1].ind1) * inv_len;
  _texdat[DAT_MID + scan].next_u = cv_sng(u);
  _texdat[DAT_MID + scan].next_v = cv_sng(v);

  _temp1 = x;

  asm volatile (
      "push %%ebp \n"
      "movl %0, %%ebp \n"

      "fild __temp1 \n" // load x
      "fldl __half \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl 88 + 52(%%ebp) \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fldl 88 + 68(%%ebp) \n"
      "fmulp %%st(0), %%st(1) \n"
      "fstpl __dtemp + 8 * 0 \n"

      "fldl 88 + 20(%%ebp) \n"
      "fldl 88 + 36(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 20(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 76(%%ebp) \n"

      "fldl 88 + 28(%%ebp) \n"
      "fldl 88 + 44(%%ebp) \n"
      "fsubp %%st(0), %%st(1) \n"
      "fldl __dtemp + 0 * 8 \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl 88 + 28(%%ebp) \n"
      "faddp %%st(0), %%st(1) \n"
      "fldl __f2sng \n"
      "fmulp %%st(0), %%st(1) \n"
      "fldl __half \n"
      "fsubrp %%st(0), %%st(1) \n"
      "fistpl 80(%%ebp) \n"

      "pop %%ebp \n"
    :
    :"g" (&_texdat[DAT_MID + scan])
    :"memory"
  );
}
*/

void draw_abs_line (line_equat line, int r, int g, int b)
{
  if (fabs(line.m) > 1) {
    double x = line.inv_b;
    for (int y = 0; y < 200; y++, x += line.inv_m)
      pxl (int(x), y, r, g, b);
  }
  else {
    double y = line.b;
    for (int x = 0; x < 320; x++, y += line.m)
      pxl (x, int(y), r, g, b);
  }
}


void draw_abs_line (line_2d line, int r, int g, int b)
{
  if (fabs(line.m) > 1) {
    double x = -line.b / line.m;
    for (int y = 0; y < 200; y++, x += 1 / line.m)
      pxl (int(x), y, r, g, b);
  }
  else {
    double y = line.b;
    for (int x = 0; x < 320; x++, y += line.m)
      pxl (x, int(y), r, g, b);
  }
}


inline line_3d calc_parll_line_3d (line_3d line, point_3d point)
{
  line_3d l3d;

  l3d.mx = line.mx;
  l3d.my = line.my;
  l3d.bx = point.x - point.z * line.mx;
  l3d.by = point.y - point.z * line.my;
  
  return l3d;
}


inline void init_horz_tri
  (point_2d p1, point_2d p2, point_2d p3, uv_calc_ref* uv_init, double scan_m)
{
  double s1 = horz_scan_b (p1, scan_m);
  double s2 = horz_scan_b (p2, scan_m);
  double s3 = horz_scan_b (p3, scan_m);
  
  double min = s1;
  if (s2 < min)
    min = s2;
  if (s3 < min)
    min = s3;
    
  double max = s1;
  if (s2 > max)
    max = s2;
  if (s3 > max)
    max = s3;

  for (int s = (int)floor(min) - 1; s <= (int)ceil(max) + 1; s++)
    if (!_texdat[s + DAT_MID].init)
      update_uv (uv_init, &_texdat[s + DAT_MID], s);
}


inline double horz_scan_b (point_2d p, double scan_m)
{
  return p.x - p.y * scan_m;
}


inline double vert_scan_b (point_2d p, double scan_m)
{
  return p.y - p.x * scan_m;
}


inline void init_vert_tri
  (point_2d p1, point_2d p2, point_2d p3, uv_calc_ref* uv_init, double scan_m)
{
  double s1 = vert_scan_b (p1, scan_m);
  double s2 = vert_scan_b (p2, scan_m);
  double s3 = vert_scan_b (p3, scan_m);
  
  double min = s1;
  if (s2 < min)
    min = s2;
  if (s3 < min)
    min = s3;
    
  double max = s1;
  if (s2 > max)
    max = s2;
  if (s3 > max)
    max = s3;

  for (int s = (int)floor(min) - 1; s <= (int)ceil(max) + 1; s++)
    if (!_texdat[s + DAT_MID].init)
      update_uv (uv_init, &_texdat[s + DAT_MID], s);
}


void check_move
  (point_3d curr, point_3d* tenat, double grav, double fric, plane_type plane,
   tri_3d tri)
{
  point_3d thru;
  double radius = 5;
  
  if (plane.y_plane)
    if (curr.y <= plane.b)
      if (tenat->y > plane.b) {
        y_line_3d l3d = calc_y_line_3d (curr, *tenat);
        thru.x = l3d.mx * plane.b + l3d.bx;
        thru.y = plane.b;
        thru.z = l3d.mz * plane.b + l3d.bz;
        if (approx_equal (tri.p1->abs.z, tri.p2->abs.z))
          if (tri.p3->abs.z > tri.p1->abs.z) { // 1-2, 3
            _dtemp[26] = 1;
            if (is_within_xz_up_tri (tri.p3->abs, tri.p2->abs, tri.p1->abs,
            thru, radius))
              tenat->y = plane.b;
          }
          else { // 3, 1-2
            _dtemp[26] = 2;
            if (is_within_xz_dn_tri (tri.p1->abs, tri.p2->abs, tri.p3->abs,
            thru, radius))
              tenat->y = plane.b;
          }
        else if (tri.p1->abs.z > tri.p2->abs.z)
          if (approx_equal (tri.p3->abs.z, tri.p1->abs.z)) { // 2, 1-3
            _dtemp[26] = 3;
            if (is_within_xz_dn_tri (tri.p3->abs, tri.p1->abs, tri.p2->abs,
            thru, radius))
              tenat->y = plane.b;
          }
          else if (approx_equal (tri.p3->abs.z, tri.p2->abs.z)) { // 2-3, 1
            _dtemp[26] = 4;
            if (is_within_xz_up_tri (tri.p1->abs, tri.p3->abs, tri.p2->abs,
            thru, radius))
              tenat->y = plane.b;
          }
          else if (tri.p3->abs.z > tri.p1->abs.z) { // 2, 1, 3
            _dtemp[26] = 5;
            if (is_within_xz_rig_tri (tri.p3->abs, tri.p1->abs, tri.p2->abs,
            thru, radius))
              tenat->y = plane.b;
          }
          else if (tri.p3->abs.z < tri.p2->abs.z) { // 3, 2, 1
            _dtemp[26] = 6;
            if (is_within_xz_rig_tri (tri.p1->abs, tri.p2->abs, tri.p3->abs,
            thru, radius))
              tenat->y = plane.b;
          }
          else { // 2, 3, 1
            _dtemp[26] = 7;
            if (is_within_xz_lef_tri (tri.p1->abs, tri.p3->abs, tri.p2->abs,
            thru, radius))
              tenat->y = plane.b;
          }
        else
          if (approx_equal (tri.p3->abs.z, tri.p1->abs.z)) { // 1-3, 2
            _dtemp[26] = 8;
            if (is_within_xz_up_tri (tri.p2->abs, tri.p1->abs, tri.p3->abs,
            thru, radius))
              tenat->y = plane.b;
          }
          else if (approx_equal (tri.p3->abs.z, tri.p2->abs.z)) { // 1, 2-3
            _dtemp[26] = 9;
           if (is_within_xz_up_tri (tri.p1->abs, tri.p3->abs, tri.p2->abs,
           thru, radius))
              tenat->y = plane.b;
          }
          else if (tri.p3->abs.z > tri.p2->abs.z) { // 1, 2, 3
            _dtemp[26] = 10;
            if (is_within_xz_lef_tri (tri.p3->abs, tri.p2->abs, tri.p1->abs,
            thru, radius))
              tenat->y = plane.b;
          }
          else if (tri.p3->abs.z < tri.p1->abs.z) { // 3, 1, 2
            _dtemp[26] = 11;
            if (is_within_xz_lef_tri (tri.p2->abs, tri.p1->abs, tri.p3->abs,
            thru, radius))
              tenat->y = plane.b;
          }
          else { // 1, 3, 2
            _dtemp[26] = 12;
            if (is_within_xz_rig_tri (tri.p2->abs, tri.p3->abs, tri.p1->abs,
            thru, radius))
              tenat->y = plane.b;
          }
      }
      else
        ;
    else
      ;
  else
    if (plane.m1_inf) {
//      if (tenat->x <= plane.m2 * tenat->y + plane.b)
//      if (curr.x - CMP_PRECISION < plane.m2 * curr.y + plane.b - radius)
//        if (tenat->x >= plane.m2 * tenat->y + plane.b - radius) {
      if (curr.x - CMP_PRECISION < plane.m2 * curr.y + plane.b - radius)
        if (tenat->x < plane.m2 * tenat->y + plane.b - radius)
          return;
      if (curr.x > plane.m2 * curr.y + plane.b)
        if (tenat->x > plane.m2 * tenat->y + plane.b)
          return;
//          point_type thru;
//inline void check_move_yz_dn_tri
// (point_3d top_lef, point_3d top_rig, point_3d bot,
//  point_3d pen, point_3d curr, point_3d* tenat,
//  double grav, double fric, double rad, plane_type plane)

          if (approx_equal (tri.p1->abs.y, tri.p2->abs.y))
            if (tri.p3->abs.y > tri.p1->abs.y) { // 1-2, 3
              check_move_yz_dn_tri (tri.p1->abs, tri.p2->abs, tri.p3->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 1;
            }
            else { // 3, 1-2
              check_move_yz_up_tri (tri.p3->abs, tri.p2->abs, tri.p1->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 2; // *
            }
          else if (tri.p1->abs.y > tri.p2->abs.y)
            if (approx_equal (tri.p3->abs.y, tri.p1->abs.y)) { // 2, 1-3
              check_move_yz_up_tri (tri.p2->abs, tri.p1->abs, tri.p3->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 3; // *
            }
            else if (approx_equal (tri.p3->abs.y, tri.p2->abs.y)) { // 2-3, 1
              check_move_yz_dn_tri (tri.p2->abs, tri.p3->abs, tri.p1->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 4; // *
            }
            else if (tri.p3->abs.y < tri.p2->abs.y) { // 3, 2, 1
              check_move_yz_lef_tri (tri.p3->abs, tri.p2->abs, tri.p1->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 5;
            }
            else if (tri.p3->abs.y > tri.p1->abs.y) { // 2, 1, 3
              check_move_yz_lef_tri (tri.p2->abs, tri.p1->abs, tri.p3->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 6;
            }
            else { // 2, 3, 1
              check_move_yz_rig_tri (tri.p2->abs, tri.p3->abs, tri.p1->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 7;
            }
          else
            if (approx_equal (tri.p3->abs.y, tri.p1->abs.y)) { // 1-3, 2
              check_move_yz_dn_tri (tri.p3->abs, tri.p1->abs, tri.p2->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 8;
            }
            else if (approx_equal (tri.p3->abs.y, tri.p2->abs.y)) { // 1, 2-3
              check_move_yz_up_tri (tri.p1->abs, tri.p3->abs, tri.p2->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 9;
            }
            else if (tri.p3->abs.y < tri.p1->abs.y) { // 3, 1, 2
              check_move_yz_rig_tri (tri.p3->abs, tri.p1->abs, tri.p2->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 10;
            }
            else if (tri.p3->abs.y > tri.p2->abs.y) { // 1, 2, 3
              check_move_yz_rig_tri (tri.p1->abs, tri.p2->abs, tri.p3->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 11;
            }
            else { // 1, 3, 2
              check_move_yz_lef_tri (tri.p1->abs, tri.p3->abs, tri.p2->abs, thru, curr, tenat, grav, fric, radius, plane);
              _dtemp[27] = 12;
            }
        }
//        else
//          ;
//      else
//        ;
    else {
      if (curr.z - CMP_PRECISION < plane.m1 * curr.x + plane.m2 * curr.y + plane.b)
        if (tenat->z > plane.m1 * tenat->x + plane.m2 * tenat->y + plane.b) {
          if (approx_equal (tenat->y, curr.y))
            if (approx_equal (tenat->x, curr.x))
              if (approx_equal (tenat->z, curr.z))
                thru = curr;
              else {
                thru.x = curr.x;
                thru.y = curr.y;
                thru.z = plane.m1 * curr.x + plane.m2 * curr.y + plane.b;
                _dtemp[26] = -101;
              }
            else {
              double b1 = plane.m2 * curr.y + plane.b;
              lin_relat path = calc_dz_dx_line (curr, *tenat);
              if (approx_equal (plane.m1, path.m))
                return;
              thru.x = (path.b - b1) / (plane.m1 - path.m);
              thru.y = curr.y;
              thru.z = path.m * thru.x + path.b;
              _dtemp[26] = -102;
            }
          else {
            y_line_3d path = calc_y_line_3d (curr, *tenat);
            if (approx_zero (path.mz - plane.m1 * path.mx - plane.m2))
              return;
            thru.y = (path.bx * plane.m1 - path.bz + plane.b) /
              (path.mz - plane.m1 * path.mx - plane.m2);
            thru.x = path.mx * thru.y + path.bx;
            thru.z = path.mz * thru.y + path.bz;
            _dtemp[26] = -103;
          }

          if (approx_equal (tri.p1->abs.y, tri.p2->abs.y))
            if (tri.p3->abs.y > tri.p1->abs.y) { // 1-2, 3
              if (is_within_xy_dn_tri
              (tri.p1->abs, tri.p2->abs, tri.p3->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 51;
            }
            else { // 3, 1-2
              if (is_within_xy_up_tri
              (tri.p3->abs, tri.p2->abs, tri.p1->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 52;
            }
          else if (tri.p1->abs.y > tri.p2->abs.y)
            if (approx_equal (tri.p3->abs.y, tri.p2->abs.y)) { // 2-3, 1
              if (is_within_xy_dn_tri
              (tri.p2->abs, tri.p3->abs, tri.p1->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 53;
            }
            else if (approx_equal (tri.p3->abs.y, tri.p1->abs.y)) { // 2, 1-3
              if (is_within_xy_up_tri
              (tri.p2->abs, tri.p1->abs, tri.p3->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 54;
            }
            else if (tri.p3->abs.y > tri.p1->abs.y) { // 2, 1, 3
              if (is_within_xy_lef_tri
              (tri.p2->abs, tri.p1->abs, tri.p3->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 55;
            }
            else if (tri.p3->abs.y < tri.p2->abs.y) { // 3, 2, 1
              if (is_within_xy_lef_tri
              (tri.p3->abs, tri.p2->abs, tri.p1->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 56;
            }
            else { // 2, 3, 1
              if (is_within_xy_rig_tri
              (tri.p2->abs, tri.p3->abs, tri.p1->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 57;
            }
          else
            if (approx_equal (tri.p3->abs.y, tri.p1->abs.y)) { // 1-3, 2
              if (is_within_xy_dn_tri
              (tri.p3->abs, tri.p1->abs, tri.p2->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 58;
            }
            else if (approx_equal (tri.p3->abs.y, tri.p2->abs.y)) { // 1, 2-3
              if (is_within_xy_up_tri
              (tri.p1->abs, tri.p3->abs, tri.p2->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 59;
            }
            else if (tri.p3->abs.y > tri.p2->abs.y) { // 1, 2, 3
              if (is_within_xy_rig_tri
              (tri.p1->abs, tri.p2->abs, tri.p3->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 60;
            }
            else if (tri.p3->abs.y < tri.p1->abs.y) { // 3, 1, 2
              if (is_within_xy_rig_tri
              (tri.p3->abs, tri.p1->abs, tri.p2->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 61;
            }
            else { // 1, 3, 2
              if (is_within_xy_lef_tri
              (tri.p1->abs, tri.p3->abs, tri.p2->abs, thru))
                deflect_xyz (thru, tenat, grav, fric, plane);
              _dtemp[26] = 62;
            }
        }
    }
}


inline bool is_within_xz_up_tri (point_3d top,
  point_3d bot_lef, point_3d bot_rig, point_3d point, double rad)
{
  if (point.z > top.z + rad)
    return false;
  else if (point.z < bot_lef.z - rad)
    return false;
  else {
    if (top.x < bot_lef.x)
      if (point.x < top.x - rad)
        return false;
      else if (point.x > bot_rig.x + rad)
        return false;
      else
        ;
    else if (top.x > bot_rig.x)
      if (point.x < bot_lef.x - rad)
        return false;
      else if (point.x > top.x + rad)
        return false;
      else
        ;
    else
      if (point.x < bot_lef.x - rad)
        return false;
      else if (point.x > bot_rig.x + rad)
        return false;
      else
        ;
    
    lin_relat lef = calc_dxdz_line (top, bot_lef); // outer left edge
    double offset_l = calc_b_offset
      (rad, top.x - bot_lef.x, top.z - bot_lef.z);
//    if (point.x < lef.m * point.z + lef.b - offset_l)
//      return false;

    lin_relat rig = calc_dxdz_line (top, bot_rig); // outer right edge
    double offset_r = calc_b_offset
      (rad, top.x - bot_rig.x, top.z - bot_rig.z);
//    if (point.x > rig.m * point.z + rig.b + offset_r)
//      return false;

    if (point.x > lef.m * point.z + lef.b) // inner triangle
      if (point.x < rig.m * point.z + rig.b)
        if (point.z < top.z)
          if (point.z > bot_lef.z)
            return true;
            
    if (point.z < bot_lef.z) // bottom rectangle
      if (point.z > bot_lef.z - rad)
        if (point.x > bot_lef.x)
          if (point.x < bot_rig.x)
            return true;
            
    if (is_within_xz_rect // left rectangle
    (top, bot_lef, lef.b - offset_l, lef.b, lef.m, point))
      return true;
    if (is_within_xz_rect // right rectangle
    (top, bot_rig, rig.b, rig.b + offset_r, rig.m, point))
      return true;
      
    if (is_within_xz_circ(top, rad, point)) // top circle
      return true;
    if (is_within_xz_circ(bot_lef, rad, point)) // bottom left circle
      return true;
    if (is_within_xz_circ(bot_rig, rad, point)) // bottom right circle
      return true;
      
    return false;
  }
}


inline bool is_within_xz_dn_tri (point_3d top_lef, point_3d top_rig,
  point_3d bot, point_3d point, double rad)
{
  if (point.z > top_lef.z + rad)
    return false;
  else if (point.z < bot.z - rad)
    return false;
  else {
    if (bot.x < top_lef.x) // exclude outside circumscribing square
      if (point.x < bot.x - rad)
        return false;
      else if (point.x > top_rig.x + rad)
        return false;
      else
        ;
    else if (bot.x > top_rig.x)
      if (point.x < top_lef.x - rad)
        return false;
      else if (point.x > bot.x + rad)
        return false;
      else
        ;
    else
      if (point.x < top_lef.x - rad)
        return false;
      else if (point.x > top_rig.x + rad)
        return false;
      else
        ;
        
    lin_relat lef = calc_dxdz_line (top_lef, bot); // outer left edge
    double offset_l = calc_b_offset
      (rad, top_lef.x - bot.x, top_lef.z - bot.z);
//    if (point.x < lef.m * point.z + lef.b - offset_l)
//      return false;

    lin_relat rig = calc_dxdz_line (top_rig, bot); // outer right edge
    double offset_r = calc_b_offset
      (rad, top_rig.x - bot.x, top_rig.z - bot.z);
//    if (point.x > rig.m * point.z + rig.b + offset_r)
//      return false;
      
    if (point.x >= lef.m * point.z + lef.b) // inner triangle
      if (point.x <= rig.m * point.z + rig.b)
        if (point.z <= top_lef.z)
          if (point.z >= bot.z)
            return true;
            
    if (point.z >= top_lef.z) // top rectangle
      if (point.z <= top_lef.z + rad)
        if (point.x >= top_lef.x)
          if (point.x <= top_rig.x)
            return true;
            
    if (is_within_xz_rect // left rectangle
    (top_lef, bot, lef.b - offset_l, lef.b, lef.m, point))
      return true;
    if (is_within_xz_rect // right rectangle
    (top_rig, bot, rig.b, rig.b + offset_r, rig.m, point))
      return true;
      
    if (is_within_xz_circ(top_lef, rad, point)) // top left circle
      return true;
    if (is_within_xz_circ(top_rig, rad, point)) // top right circle
      return true;
    if (is_within_xz_circ(bot, rad, point)) // bottom circle
      return true;
      
    return false;
  }
}


inline bool is_within_xz_lef_tri (point_3d top, point_3d mid,
  point_3d bot, point_3d point, double rad)
{
  if (point.z > top.z + rad)
    return false;
  else if (point.z < bot.z - rad)
    return false;
  else {
    if (top.x < mid.x) // exclude outside circumscribing square
      if (bot.x < top.x) // b,t,m
        if (point.x < bot.x - rad)
          return false;
        else if (point.x > mid.x + rad)
          return false;
        else
          ;
      else if (bot.x > mid.x) // t,m,b
        if (point.x < top.x - rad)
          return false;
        else if (point.x > bot.x + rad)
          return false;
        else
          ;
      else // t,b,m
        if (point.x < top.x - rad)
          return false;
        else if (point.x > mid.x + rad)
          return false;
        else
          ;
    else
      if (bot.x < mid.x) // b,m,t
        if (point.x < bot.x - rad)
          return false;
        else if (point.x > top.x + rad)
          return false;
        else
          ;
      else if (bot.x > top.x) // m,t,b
        if (point.x < mid.x - rad)
          return false;
        else if (point.x > bot.x + rad)
          return false;
        else
          ;
      else // m,b,t
        if (point.x < mid.x - rad)
          return false;
        else if (point.x > top.x + rad)
          return false;
        else
          ;
    
    lin_relat lt = calc_dxdz_line (top, mid); // top left line
    double offset_lt = calc_b_offset (rad, top.x - mid.x, top.z - mid.z);
//    if (point.x < point.z * lt.m + lt.b - offset_lt)
//      return false;

    lin_relat lb = calc_dxdz_line (mid, bot); // bottom left line
    double offset_lb = calc_b_offset (rad, bot.x - mid.x, bot.z - mid.z);
//    if (point.x < point.z * lb.m + lb.b - offset_lb)
//      return false;
    
    lin_relat r = calc_dxdz_line (top, bot); // right line
    double offset_r = calc_b_offset (rad, top.x - bot.x, top.z - bot.z);
//    if (point.x > point.z * r.m + r.b + offset_r)
//      return false;

    if (point.x >= point.z * lt.m + lt.b) // inner triangle
      if (point.x >= point.z * lb.m + lb.b)
        if (point.x <= point.z * r.m + r.b)
          return true;

//  (point_3d p1, point_3d p2, double b_lef, double b_rig, double m,
//   point_3d point)

    if (is_within_xz_rect // top left rectangle
    (top, mid, lt.b - offset_lt, lt.b, lt.m, point))
      return true;
    if (is_within_xz_rect // bot left rectangle
    (mid, bot, lb.b - offset_lb, lb.b, lb.m, point))
      return true;
    if (is_within_xz_rect // right rectangle
    (top, bot, r.b, r.b + offset_r, r.m, point))
      return true;
      
    if (is_within_xz_circ(top, rad, point)) // top circle
      return true;
    if (is_within_xz_circ(mid, rad, point)) // middle circle
      return true;
    if (is_within_xz_circ(bot, rad, point)) // bottom circle
      return true;

    return false;
  }
  
  /*
  else if (approx_equal (point.z, mid.z)) {
    line_3d rig = calc_line_3d (top, bot);
    if (point.x < mid.x)
      return false;
    else if (point.x > mid.z * rig.mx + rig.bx)
      return false;
    else
      return true;
  }
  else if (point.z > mid.z) {
    line_3d lef = calc_line_3d (top, mid);
    line_3d rig = calc_line_3d (top, bot);
    if (point.x < point.z * lef.mx + lef.bx)
      return false;
    else if (point.x > point.z * rig.mx + rig.bx)
      return false;
    else
      return true;
  }
  else {
    line_3d lef = calc_line_3d (mid, bot);
    line_3d rig = calc_line_3d (top, bot);
    if (point.x < point.z * lef.mx + lef.bx)
      return false;
    else if (point.x > point.z * rig.mx + rig.bx)
      return false;
    else
      return true;
  }
*/
}


inline bool is_within_xz_rig_tri (point_3d top, point_3d mid,
  point_3d bot, point_3d point, double rad)
{
  if (point.z > top.z)
    return false;
  else if (point.z < bot.z)
    return false;
  else {
    if (top.x < mid.x) // exclude outside circumscribing square
      if (bot.x < top.x) // b,t,m
        if (point.x < bot.x - rad)
          return false;
        else if (point.x > mid.x + rad)
          return false;
        else
          ;
      else if (bot.x > mid.x) // t,m,b
        if (point.x < top.x - rad)
          return false;
        else if (point.x > bot.x + rad)
          return false;
        else
          ;
      else // t,b,m
        if (point.x < top.x - rad)
          return false;
        else if (point.x > mid.x + rad)
          return false;
        else
          ;
    else
      if (bot.x < mid.x) // b,m,t
        if (point.x < bot.x - rad)
          return false;
        else if (point.x > top.x + rad)
          return false;
        else
          ;
      else if (bot.x > top.x) // m,t,b
        if (point.x < mid.x - rad)
          return false;
        else if (point.x > bot.x + rad)
          return false;
        else
          ;
      else // m,b,t
        if (point.x < mid.x - rad)
          return false;
        else if (point.x > top.x + rad)
          return false;
        else
          ;

    lin_relat l = calc_dxdz_line (top, bot); // left line
    double offset_l = calc_b_offset (rad, top.x - bot.x, top.z - bot.z);
    lin_relat rt = calc_dxdz_line (top, mid); // top right line
    double offset_rt = calc_b_offset (rad, top.x - mid.x, top.z - mid.z);
    lin_relat rb = calc_dxdz_line (mid, bot); // bot right line
    double offset_rb = calc_b_offset (rad, mid.x - bot.x, mid.z - bot.z);

    if (point.x >= point.z * l.m + l.b) // inner triangle
      if (point.x <= point.z * rt.m + rt.b)
        if (point.x <= point.z * rb.m + rb.b)
          return true;

    if (is_within_xz_rect // left rectangle
    (top, bot, l.b - offset_l, l.b, l.m, point))
      return true;
    if (is_within_xz_rect // top right rectangle
    (top, mid, rt.b, rt.b + offset_rt, rt.m, point))
      return true;
    if (is_within_xz_rect // bot right rectangle
    (mid, bot, rb.b, rb.b + offset_rb, rb.m, point))
      return true;
      
    if (is_within_xz_circ(top, rad, point)) // top circle
      return true;
    if (is_within_xz_circ(mid, rad, point)) // middle circle
      return true;
    if (is_within_xz_circ(bot, rad, point)) // bottom circle
      return true;

    return false;
  }
}


inline void check_move_yz_up_tri
 (point_3d top, point_3d bot_lef, point_3d bot_rig,
  point_3d pen, point_3d curr, point_3d* tenat,
  double grav, double fric, double rad, plane_type plane)
{
  if (!(curr.x - CMP_PRECISION < plane.m2 * curr.y + plane.b - rad))
    return;
  if (!pen_yz_plane (curr, *tenat, plane, rad, &pen))
    return;

  if (pen.y >= top.y)
    if (pen.y <= bot_lef.y) {
      y_line_3d l1 = calc_y_line_3d (top, bot_lef);
      if (pen.z > l1.mz * pen.y + l1.bz)
        return;
      y_line_3d l2 = calc_y_line_3d (top, bot_rig);
      if (pen.z < l2.mz * pen.y + l2.bz)
        return;
      deflect_xy (pen, tenat, grav, fric, rad, plane);
    }
}


inline void check_move_yz_dn_tri
 (point_3d top_lef, point_3d top_rig, point_3d bot,
  point_3d pen, point_3d curr, point_3d* tenat,
  double grav, double fric, double rad, plane_type plane)
{
  if (curr.y <= top_lef.y)
    if (tenat->y > top_lef.y) {
      y_line_3d path = calc_y_line_3d (curr, *tenat);
      point_3d pen;
      pen.x = path.mx * top_lef.y + path.bx;
      pen.z = path.mz * top_lef.y + path.bz;
      if (pen.x > top_lef.x - rad)
        if (pen.x < top_lef.x)
          if (pen.z > top_rig.z)
            if (pen.z < top_lef.z)
              tenat->y = top_lef.y;
      if (is_within_xz_circ (top_lef, rad, pen))
        tenat->y = top_lef.y;
      if (is_within_xz_circ (top_rig, rad, pen))
        tenat->y = top_lef.y;
    }
  else if (curr.y >= bot.y)
    if (tenat->y < bot.y) {
      y_line_3d path = calc_y_line_3d (curr, *tenat);
      point_3d pen;
      pen.x = path.mx * bot.y + path.bx;
      pen.z = path.mz * bot.y + path.bz;
      if (is_within_xz_circ (bot, rad, pen))
        tenat->y = bot.y;
    }    
  else
    ;
    
    if (curr.x - CMP_PRECISION <= plane.m2 * curr.y + plane.b - rad)
      if (pen_yz_plane (curr, *tenat, plane, rad, &pen))
        if (pen.y >= top_lef.y) // pen.y
          if (pen.y <= bot.y) { // pen.y
            y_line_3d l1 = calc_y_line_3d (bot, top_lef);
            y_line_3d l2 = calc_y_line_3d (bot, top_rig);
            if (pen.z <= l1.mz * pen.y + l1.bz)
              if (pen.z >= l2.mz * pen.y + l2.bz)
                deflect_xy (pen, tenat, grav, fric, rad, plane);
          }

  y_line_3d lef_line = calc_y_line_3d (bot, top_lef);
  y_line_3d rig_line = calc_y_line_3d (bot, top_rig);
  colis_cylinder (lef_line, top_lef, bot, rad, curr, tenat, grav, fric);
  colis_cylinder (rig_line, top_rig, bot, rad, curr, tenat, grav, fric);
}


inline void check_move_yz_lef_tri
  (point_3d top, point_3d mid, point_3d bot,
   point_3d pen, point_3d curr, point_3d* tenat,
   double grav, double fric, double rad, plane_type plane)
{
  if (curr.x - CMP_PRECISION < plane.m2 * curr.y + plane.b - rad)
    if (pen_yz_plane (curr, *tenat, plane, rad, &pen))
     if (pen.y < top.y)
       return;
     else if (pen.y > bot.y)
       return;
     else {
       y_line_3d top_lef = calc_y_line_3d (top, mid);
       if (pen.z > pen.y * top_lef.mz + top_lef.bz)
         return;
       y_line_3d bot_lef = calc_y_line_3d (mid, bot);
       if (pen.z > pen.y * bot_lef.mz + bot_lef.bz)
         return;
       y_line_3d rig = calc_y_line_3d (top, bot);
       if (pen.z < pen.y * rig.mz + rig.bz)
         return;
       deflect_xy (pen, tenat, grav, fric, rad, plane);
     }
}


inline void check_move_yz_rig_tri
  (point_3d top, point_3d mid, point_3d bot,
   point_3d pen, point_3d curr, point_3d* tenat,
   double grav, double fric, double rad, plane_type plane)
{
  if (!(curr.x - CMP_PRECISION < plane.m2 * curr.y + plane.b - rad))
    return;
  if (!pen_yz_plane (curr, *tenat, plane, rad, &pen))
    return;

  if (pen.y < top.y)
    return;
  else if (pen.y > bot.y)
    return;
  else {
    y_line_3d lef = calc_y_line_3d (top, bot);
    if (pen.z > mid.y * lef.mz + lef.bz)
      return;
    y_line_3d top_rig = calc_y_line_3d (top, mid);
    if (pen.z < pen.y * top_rig.mz + top_rig.bz)
      return;
    y_line_3d bot_rig = calc_y_line_3d (mid, bot);
    if (pen.z < pen.y * bot_rig.mz + bot_rig.bz)
      return;
    deflect_xy (pen, tenat, grav, fric, rad, plane);
  }
}


inline bool is_within_xy_up_tri
  (point_3d top, point_3d bot_lef, point_3d bot_rig, point_3d point)
{
  if (point.y < top.y)
    return false;
  else if (point.y > bot_lef.y)
    return false;
  else {
    y_line_3d l1 = calc_y_line_3d (top, bot_lef);
    y_line_3d l2 = calc_y_line_3d (top, bot_rig);
    if (point.x < l1.mx * point.y + l1.bx)
      return false;
    else if (point.x > l2.mx * point.y + l2.bx)
       return false;
    else
      return true;
  }
}


inline bool is_within_xy_dn_tri
  (point_3d top_lef, point_3d top_rig, point_3d bot, point_3d point)
{
  if (point.y < top_lef.y)
    return false;
  else if (point.y > bot.y)
    return false;
  else {
    y_line_3d l1 = calc_y_line_3d (top_lef, bot);
    y_line_3d l2 = calc_y_line_3d (top_rig, bot);
    if (point.x < l1.mx * point.y + l1.bx)
      return false;
    else if (point.x > l2.mx * point.y + l2.bx)
       return false;
    else
      return true;
  }
}


inline bool is_within_xy_lef_tri
  (point_3d top, point_3d mid, point_3d bot, point_3d point)
{
  if (point.y < top.y)
    return false;
  else if (point.y > bot.y)
    return false;
  else if (approx_equal (point.y, mid.y)) {
    y_line_3d rig = calc_y_line_3d (top, bot);
    if (point.x < mid.x)
      return false;
    else if (point.x > mid.y * rig.mx + rig.bx)
      return false;
    else
      return true;
  }
  else if (point.y < mid.y) {
    y_line_3d lef = calc_y_line_3d (top, mid);
    y_line_3d rig = calc_y_line_3d (top, bot);
    if (point.x < point.y * lef.mx + lef.bx)
      return false;
    else if (point.x > point.y * rig.mx + rig.bx)
      return false;
    else
      return true;
  }
  else {
    y_line_3d lef = calc_y_line_3d (mid, bot);
    y_line_3d rig = calc_y_line_3d (top, bot);
    if (point.x < point.y * lef.mx + lef.bx)
      return false;
    else if (point.x > point.y * rig.mx + rig.bx)
      return false;
    else
      return true;
  }
}


inline bool is_within_xy_rig_tri
  (point_3d top, point_3d mid, point_3d bot, point_3d point)
{
  if (point.y < top.y)
    return false;
  else if (point.y > bot.y)
    return false;
  else if (approx_equal (point.y, mid.y)) {
    y_line_3d lef = calc_y_line_3d (top, bot);
    if (point.x > mid.x)
      return false;
    else if (point.x < mid.y * lef.mx + lef.bx)
      return false;
    else
      return true;
  }
  else if (point.y < mid.y) {
    y_line_3d lef = calc_y_line_3d (top, bot);
    y_line_3d rig = calc_y_line_3d (top, mid);
    if (point.x < point.y * lef.mx + lef.bx)
      return false;
    else if (point.x > point.y * rig.mx + rig.bx)
      return false;
    else
      return true;
  }
  else {
    y_line_3d lef = calc_y_line_3d (top, bot);
    y_line_3d rig = calc_y_line_3d (mid, bot);
    if (point.x < point.y * lef.mx + lef.bx)
      return false;
    else if (point.x > point.y * rig.mx + rig.bx)
      return false;
    else
      return true;
  }
}


inline void deflect_xy
  (point_3d thru, point_3d* point, double grav, double fric, double rad,
  plane_type plane)
{
  if (_play_mode)
    if (grav - grav * fric * fabs(plane.m2) <= 0)
      point->y = thru.y;
    else
      point->y = thru.y + (grav - grav * fric * fabs(plane.m2));

  double b2 = point->x - plane.perp_m2 * point->y;
  point->y = (b2 - (plane.b - rad)) / (plane.m2 - plane.perp_m2);
  point->x = plane.m2 * point->y + plane.b - rad;
}


inline void deflect_xyz
  (point_3d thru, point_3d* point, double grav, double fric, plane_type plane)
{
  double m = xyz_plane_fall_m (plane);

  if (grav - grav * fric * m <= 0)
    point->y = thru.y;
  else
    point->y = thru.y + (grav - grav * fric * m);
    
  double b1 = point->x + plane.m1 * point->z;
  double b2 = point->y + plane.m2 * point->z;
  point->z = (plane.m1 * b1 + plane.m2 * b2 + plane.b) /
    (1 + plane.m1 * plane.m1 + plane.m2 * plane.m2);
  point->x = b1 - plane.m1 * point->z;
  point->y = b2 - plane.m2 * point->z;
}


inline double xyz_plane_fall_m (plane_type plane)
{
  return fabs(plane.m2 / sin(atan(plane.m1) + PI / 2));
}


inline point_3d offset_point_3d
  (point_3d point, double x, double y, double z)
{
  point_3d temp;
  
  temp.x = point.x + x;
  temp.y = point.y + y;
  temp.z = point.z + z;

  return temp;
}


inline bool approx_zero (double n)
{
  return (fabs(n) < CMP_PRECISION);
}


inline double dz_dx (point_3d p1, point_3d p2)
{
  return (p2.z - p1.z) / (p2.x - p1.x);
}


inline lin_relat calc_dz_dx_line (point_3d p1, point_3d p2)
{
  lin_relat line;

  line.m = dz_dx (p1, p2);
  line.b = p1.z - p1.x * line.m;
  
  return line;
}


/*
bool point_collis_tri
  (point_3d curr, point_3d tenat, plane_type plane, tri_3d tri)
{
  point_3d thru;
  
  if (plane.y_plane)
    ;
  else if (plane.m1_inf) {
    if (curr.x - CMP_PRECISION < plane.m2 * curr.y + plane.b)
      if (tenat.x > plane.m2 * tenat.y + plane.b)
        if (approx_equal (curr.y, tenat.y))
          if (approx_equal (curr.z, tenat.z))
            if (approx_equal (curr.x, tenat.x))
              thru = curr;
            else {
              thru.x = plane.m2 * curr.y + plane.b;
              thru.y = curr.y;
              thru.z = curr.z;
            }
          else {
            line_3d zline = calc_line_3d (curr, tenat);
            lin_relat line;
            line.m = (curr.z - tenat.z) / (curr.x - tenat.x);
            line.b = curr.z - curr.x * line.m;
            thru.x = zline.mx * curr.y + zline.bx;
            thru.y = curr.y;
            thru.z = line.m * thru.x + line.b;
          }
        else {
          y_line_3d yline = calc_y_line_3d (curr, tenat);
          thru.y = (yline.bx - plane.b) / (plane.m2 - yline.mx);
          thru.x = yline.mx * thru.y + yline.bx;
          thru.z = yline.mz * thru.y + yline.bz;
        }
      else if (approx_equal (tenat.x, plane.m2 * tenat.y + plane.b))
        thru = tenat;
      else
        return false;
    else
      return false;

    return point_is_in_plane (thru, plane, tri);
  }
  else
    ;
}
*/

/*
bool point_is_on_tri (point_3d point, plane_type plane, tri_3d tri)
{
  if (plane.y_plane)
    ;
  else if (plane.m1_inf)
    if (fabs(point.x - plane.m2 * point.y + plane.b) < .0001)
      return point_is_in_plane (point, plane, tri);
    else
      return false;
  else
    ;
}
*/

/*
inline bool point_is_in_plane (point_3d point, plane_type plane, tri_3d tri)
{
  if (approx_equal (tri.p1->abs.y, tri.p2->abs.y))
    if (tri.p3->abs.y > tri.p1->abs.y) // 1-2, 3
      return is_within_yz_tri (tri.p1->abs, tri.p3->abs,
        tri.p1->abs, tri.p3->abs, tri.p2->abs, tri.p3->abs, point);
      else // 3, 1-2
        return is_within_yz_tri (tri.p3->abs, tri.p1->abs,
          tri.p3->abs, tri.p2->abs, tri.p3->abs, tri.p1->abs, point);
    else if (tri.p1->abs.y > tri.p2->abs.y)
      if (approx_equal (tri.p3->abs.y, tri.p1->abs.y)) // 2, 1-3
        return is_within_yz_tri (tri.p2->abs, tri.p1->abs,
          tri.p2->abs, tri.p1->abs, tri.p2->abs, tri.p3->abs, point);
      else if (approx_equal (tri.p3->abs.y, tri.p2->abs.y)) // 2-3, 1
        return is_within_yz_tri (tri.p2->abs, tri.p1->abs,
          tri.p2->abs, tri.p1->abs, tri.p3->abs, tri.p1->abs, point);
      else if (tri.p3->abs.y < tri.p2->abs.y) // 3, 2, 1
        return is_within_yz_lef_tri
          (tri.p3->abs, tri.p2->abs, tri.p1->abs, point);
      else if (tri.p3->abs.y > tri.p1->abs.y) // 2, 1, 3
        return is_within_yz_lef_tri
          (tri.p2->abs, tri.p1->abs, tri.p3->abs, point);
      else // 2, 3, 1
        return is_within_yz_rig_tri
          (tri.p2->abs, tri.p3->abs, tri.p1->abs, point);
    else
      if (approx_equal (tri.p3->abs.y, tri.p1->abs.y)) // 1-3, 2
        return is_within_yz_tri (tri.p1->abs, tri.p2->abs,
          tri.p3->abs, tri.p2->abs, tri.p1->abs, tri.p2->abs, point);
      else if (approx_equal (tri.p3->abs.y, tri.p2->abs.y)) // 1, 2-3
        return is_within_yz_tri (tri.p1->abs, tri.p2->abs,
          tri.p1->abs, tri.p3->abs, tri.p1->abs, tri.p2->abs, point);
      else if (tri.p3->abs.y < tri.p1->abs.y) // 3, 1, 2
        return is_within_yz_rig_tri
          (tri.p3->abs, tri.p1->abs, tri.p2->abs, point);
      else if (tri.p3->abs.y > tri.p2->abs.y) // 1, 2, 3
        return is_within_yz_rig_tri
          (tri.p1->abs, tri.p2->abs, tri.p3->abs, point);
      else // 1, 3, 2
        return is_within_yz_lef_tri
          (tri.p1->abs, tri.p3->abs, tri.p2->abs, point);
}
*/

inline double calc_b_offset (double rad, double dx, double dy)
{
  return fabs(rad / sin(atan2(dy, dx)));
}


inline lin_relat calc_dxdz_line (point_3d p1, point_3d p2)
{
  lin_relat line;

  line.m = (p2.x - p1.x) / (p2.z - p1.z);
  line.b = p1.x - p1.z * line.m;
  
  return line;
}


inline lin_relat calc_dzdx_line (point_3d p1, point_3d p2)
{
  lin_relat line;

  line.m = (p2.z - p1.z) / (p2.x - p1.x);
  line.b = p1.z - p1.x * line.m;
  
  return line;
}


inline bool is_within_xz_rect
  (point_3d p1, point_3d p2, double b_lef, double b_rig, double m,
   point_3d point)
{
  if (point.x < m * point.z + b_lef)
    return false;
  else if (point.x > m * point.z + b_rig)
    return false;
  else { //return true;
    if (is_approx_zero (m))
      if (point.z > p1.z)
        return false;
      else if (point.z < p2.z)
        return false;
      else
        return true;
    else {
      double perp_m = -1 / m;
      double b_top = p1.z - p1.x * perp_m;
      if (point.z > perp_m * point.x + b_top)
        return false;
      double b_bot = p2.z - p2.x * perp_m;
      if (point.z < perp_m * point.x + b_bot)
        return false;
      return true;
    }
  }
}


inline bool is_within_xz_circ (point_3d cen, double rad, point_3d point)
{
  double dist = sqrt((cen.x - point.x) * (cen.x - point.x) +
                     (cen.z - point.z) * (cen.z - point.z));
  return (dist <= rad);
}


inline bool pen_yz_plane
  (point_3d curr, point_3d tenat, plane_type plane,
   double radius, point_3d* pen)
{
  if (approx_equal (curr.y, tenat.y))
    if (approx_equal (curr.z, tenat.z))
      if (approx_equal (curr.x, tenat.x)) {
        *pen = curr;
        _dtemp[26] = -1; // all equal
        return true;
      }
      else {
        pen->x = plane.m2 * curr.y + plane.b - radius;
        pen->y = curr.y;
        pen->z = curr.z;
        _dtemp[26] = -2; // all equal except curr.x != tenat.x
        return true;
      }
      else {
        if (approx_equal (curr.x, tenat.x))
          return false;
        else {
          lin_relat path = calc_dzdx_line (curr, tenat);
          pen->x = plane.m2 + curr.y + plane.b - radius;
          pen->y = curr.y;
          pen->z = path.m * curr.x + path.b;
          _dtemp[26] = -3; // not x, not z, y
          return true;
        }
      }
      else {
        y_line_3d yline = calc_y_line_3d (curr, tenat);
        if (approx_equal (plane.m2, yline.mx))
          return false;
        else {
          pen->y = (yline.bx - (plane.b - radius)) / (plane.m2 - yline.mx);
          pen->x = yline.mx * pen->y + yline.bx;
          pen->z = yline.mz * pen->y + yline.bz;
          return true;
        }
     }
}


inline bool is_in_y_cylinder
  (point_3d p1, point_3d p2, double rad, point_3d point)
{
  y_line_3d axis = calc_y_line_3d (p1, p2);
  point_3d cent;
  cent.x = axis.mx * point.y + axis.bx;
  cent.z = axis.mz * point.y + axis.bz;
  return is_within_xz_circ (cent, rad, point);
}


inline bool quadratic
  (double a, double b, double c, double* r1, double* r2)
{
  double x = sqrt(b * b - 4 * a * c);
  double den = .5 / a;
  *r1 = (x - b) * den;
  *r2 = -(x + b) * den;
  return b * b - 4 * a * c >= 0;
}


inline double path_int_cyl
  (point_3d inside, point_3d outside, y_line_3d cent, double rad)
{
  y_line_3d path = calc_y_line_3d (inside, outside);
  double r1, r2;
  
  quadratic (
    path.mx * path.mx - 2 * path.mx * cent.mx + cent.mx * cent.mx +
    path.mz * path.mz - 2 * path.mz * cent.mz + cent.mz * cent.mz,
    2 * (path.mx * path.bx - cent.mx * path.bx + cent.mx * cent.bx - path.mx * cent.bx +
         path.mz * path.bz - cent.mz * path.bz + cent.mz * cent.bz - path.mz * cent.bz),
    path.bx * path.bx - 2 * path.bx * cent.bx + cent.bx * cent.bx +
    path.bz * path.bz - 2 * path.bz * cent.bz + cent.bz * cent.bz - rad * rad,
    &r1, &r2
  );

  if (inside.y < outside.y)
    return r1;
  else
    return r2;
}


inline point_3d path_int_cyl_point
  (point_3d inside, point_3d outside, y_line_3d cent, double rad)
{
  y_line_3d path = calc_y_line_3d (inside, outside);
  point_3d temp;
  double r1, r2;
  
  quadratic (
    path.mx * path.mx - 2 * path.mx * cent.mx + cent.mx * cent.mx +
    path.mz * path.mz - 2 * path.mz * cent.mz + cent.mz * cent.mz,
    2 * (path.mx * path.bx - cent.mx * path.bx + cent.mx * cent.bx - path.mx * cent.bx +
         path.mz * path.bz - cent.mz * path.bz + cent.mz * cent.bz - path.mz * cent.bz),
    path.bx * path.bx - 2 * path.bx * cent.bx + cent.bx * cent.bx +
    path.bz * path.bz - 2 * path.bz * cent.bz + cent.bz * cent.bz - rad * rad,
    &r1, &r2
  );

  if (inside.y < outside.y)
    temp.y = r1;
  else
    temp.y = r2;

  temp.x = path.mx * temp.y + path.bx;
  temp.z = path.mz * temp.y + path.bz;
  return temp;
}


inline point_3d point_perp_line (y_line_3d line, point_3d point)
{
  point_3d temp;

  temp.y = (line.mx * point.x + line.mz * point.z + point.y -
           line.mx * line.bx - line.mz * line.bz) /
           (line.mx * line.mx + line.mz * line.mz + 1);
  temp.x = line.mx * temp.y + line.bx;
  temp.z = line.mz * temp.y + line.bz;

  return temp;
}


inline void colis_cylinder
  (y_line_3d center, point_3d top, point_3d bot, double rad,
   point_3d curr, point_3d* tenat, double grav, double fric)
{
  double rad2 = rad * 1.01;//CMP_PRECISION * 2;
  
    if (!is_in_y_cylinder (top, bot, rad - 0 * CMP_PRECISION, curr))
      if (is_in_y_cylinder (top, bot, rad - 0 * CMP_PRECISION, *tenat))
        if (approx_equal (tenat->y, curr.y)) {
          if (curr.y > top.y)
            if (curr.y < bot.y) {
              double r1, r2;
              point_3d cent;
              cent.x = center.mx * curr.y + center.bx;
              cent.z = center.mz * curr.y + center.bz;
              lin_relat line = calc_dzdx_line (*tenat, cent);
              quadratic (1 + line.m * line.m,
                         2 * (line.m * line.b - line.m * cent.z - cent.x),
                         line.b * line.b - 2 * line.b * cent.z +
                         cent.z * cent.z + cent.x * cent.x - rad2 * rad2,
                         &r1, &r2);
              tenat->x = r2;
              tenat->z = line.m * r2 + line.b;
            }
        }
        else {
          double r1, r2;
          point_3d ref;
          double m = edge_fall_m (top, bot);

          if (grav - grav * fric * m <= 0)
            ref.y = path_int_cyl (*tenat, curr, center, rad2);
          else
            ref.y = path_int_cyl (*tenat, curr, center, rad2) +
                    (grav - grav * fric * m);

          ref.x = tenat->x;
          ref.z = tenat->z;
          point_3d closest = point_perp_line (center, ref);
          y_line_3d perp = calc_y_line_3d (closest, ref);
          *tenat = path_int_cyl_point (closest, ref, center, rad2);
        }
}


inline double edge_fall_m (point_3d top, point_3d bot)
{
  double dx = bot.x - top.x;
  double dz = bot.z - top.z;
  
  return sqrt(dx * dx + dz * dz) / (bot.y - top.y);
}


inline y_line_3d parll_y_line_3d (y_line_3d line, point_3d point)
{
  y_line_3d temp;

  temp.mx = line.mx;
  temp.mz = line.mz;
  temp.bx = point.x - point.y * temp.mx;
  temp.bz = point.z - point.y * temp.mz;

  return temp;
}
