#define FULL_ROTATE 4000

inline double units_to_rad (double units);

struct point_type {
  point_3d abs, rel;
  point_2d scr;
  bool select;
};

struct tri_3d {
  point_type* p1;
  point_type* p2;
  point_type* p3;
};

struct rot_data {
  double m;
  double x_sub, y_sub;
  double inv_dm;
};

struct camera_type {
  point_3d pos;
  double angle_xz;
  double angle_yz;
  double angle_xy;
};

struct z_map_type {
  unsigned long inv_z;
  long poly;
};

struct view_type {
  camera_type camera;
  double z_cutoff;
  int logical_width;
  point_2d center;
  dbl_pair zoom;
  dbl_pair window[2];
  line_equat edge[4];
  z_map_type* z_map;
  void* write_to;
  int vis_page;
  rot_data rot_xz, rot_yz, rot_xy;
  double vis_lm, vis_rm, vis_tm, vis_bm;
};

struct tri_dat_type{

};

struct tri_type {
  tri_3d t3d;
  true_clr bk_light;
  tri_dat_type* dat;
};

class tri_group {
  tri_type* first;
  tri_type* last;
  tex_ref ref;
  plane_type plane;
  int vis_side;
};

inline void calc_rel_plane (plane_type* plane, point_type* point);
inline void calc_abs_plane (plane_type* plane, point_type* point);

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


inline void calc_rot_data (rot_data* dat, double a)
{
  dat->m = -1 / tan(a);
  dat->inv_dm = 1 / (dat->m - tan(a));
  dat->x_sub = sin(a) - dat->m * cos(a);
  dat->y_sub = sin(a + PI * .5) - tan(a) * cos(a + PI * .5);
}


inline void calc_rel_3d_points
  (point_3d center,
   double angle_xz, double angle_yz, double angle_xy,
   point_type* p, point_type* last)
{
  for (point_type* i = p; i <= last; i++) {
    i->rel.x = i->abs.x - center.x;
    i->rel.y = i->abs.y - center.y;
    i->rel.z = i->abs.z - center.z;
  }

  if (!approx_equal(angle_xz, 0))
    if (approx_equal(angle_xz, 1 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.x;
        i->rel.x = -i->rel.z;
        i->rel.z = temp;
      }
    else if (approx_equal(angle_xz, 2 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        i->rel.x = -i->rel.x;
        i->rel.z = -i->rel.z;
      }
    else if (approx_equal(angle_xz, 3 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.x;
        i->rel.x = i->rel.z;
        i->rel.z = -temp;
      }
    else {
      rot_data rot;
      calc_rot_data (&rot, angle_xz);
      for (point_type* i = p; i <= last; i++)
        group_rot (rot, &i->rel.x, &i->rel.z);
    }

  if (!approx_equal(angle_yz, 0))
    if (approx_equal(angle_yz, 1 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.z;
        i->rel.z = -i->rel.y;
        i->rel.y = temp;
      }
    else if (approx_equal(angle_yz, 2 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        i->rel.z = -i->rel.z;
        i->rel.y = -i->rel.y;
      }
    else if (approx_equal(angle_yz, 3 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.z;
        i->rel.z = i->rel.y;
        i->rel.y = -temp;
      }
    else {
      rot_data rot;
      calc_rot_data (&rot, angle_yz);
      for (point_type* i = p; i <= last; i++)
        group_rot (rot, &i->rel.z, &i->rel.y);
    }

  if (!approx_equal(angle_xy, 0))
    if (approx_equal(angle_xy, 1 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.x;
        i->rel.x = -i->rel.y;
        i->rel.y = temp;
      }
    else if (approx_equal(angle_xy, 2 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        i->rel.x = -i->rel.x;
        i->rel.y = -i->rel.y;
      }
    else if (approx_equal(angle_xy, 3 * PI / 2))
      for (point_type* i = p; i <= last; i++) {
        double temp = i->rel.x;
        i->rel.x = i->rel.y;
        i->rel.y = -temp;
      }
    else {
      rot_data rot;
      calc_rot_data (&rot, angle_xy);
      for (point_type* i = p; i <= last; i++)
        group_rot (rot, &i->rel.x, &i->rel.y);
    }
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


void clear_screen()
{
  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++)
      pxl (x, y, 0, 0, 0);
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


inline double find_vert_scan_b (point_2d point, double scan_m)
{
  return point.x - point.y * scan_m;
}


inline double find_horz_scan_b (point_2d point, double scan_m)
{
  return point.y - point.x * scan_m;
}


inline void calc_abs_plane (plane_type* plane, point_type* point)
{
  calc_plane (plane, point[0].abs, point[1].abs, point[2].abs);
}


inline void calc_rel_plane (plane_type* plane, point_type* point)
{
  calc_plane (plane, point[0].rel, point[1].rel, point[2].rel);
}


inline line_3d calc_aim_line (camera_type ent)
{
  line_3d line;

  line.mx = tan(ent.angle_xz);
  line.my = tan(ent.angle_yz);
  line.bx = ent.pos.x - line.mx * ent.pos.z;
  line.by = ent.pos.y - line.my * ent.pos.z;
  
  return line;
}


inline double units_to_rad (double units)
{
  return (units * PI * 2) / (FULL_ROTATE);
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

