#include <math.h>
#include <c:/programs/zmap/math2d.h>

#ifndef INCLUDE_MATH_3D
#define INCLUDE_MATH_3D

#ifndef CMP_PRECISION
#define CMP_PRECISION (1.0 / 1000000.0)
#endif

struct point_3d {
  double x, y, z;
};

struct line_3d {
  double mx, my, bx, by;
};

struct plane_type {
// z = m1 * x + b1
// b1 = m2 * y + b
  double m1, m2, b;
  double perp_m1, perp_m2;
  bool y_plane;
  bool m1_inf;
  double sec_m1;
};

struct x_line_3d {
  double my, mz, by, bz;
};

struct y_line_3d {
  double mx, mz, bx, bz;
};

inline line_3d calc_line_3d (point_3d p1, point_3d p2);
inline point_3d intrapolate_3d (point_3d p1, point_3d p2, double z);
inline void calc_line_3d (line_3d* l3d, point_3d p1, point_3d p2);
inline point_3d intersect_line_plane_3d (plane_type plane, line_3d line);
inline x_line_3d calc_x_line_3d (point_3d p1, point_3d p2);
inline y_line_3d calc_y_line_3d (point_3d p1, point_3d p2);
inline void calc_y_line_3d (y_line_3d* line, point_3d p1, point_3d p2);
inline int point_rel_to_plane (plane_type plane, point_3d point, double rad);
inline int point_relative_to_plane (plane_type plane, point_3d point);
void calc_plane
  (plane_type* plane, point_3d point1, point_3d point2, point_3d point3);
inline y_line_3d parll_y_line_3d (y_line_3d line, point_3d point);
inline point_3d point_perp_line (y_line_3d line, point_3d point);
inline lin_relat calc_dz_dx_line (point_3d p1, point_3d p2);
inline double dz_dx (point_3d p1, point_3d p2);
inline point_3d offset_point_3d
  (point_3d point, double x, double y, double z);
inline line_3d calc_parll_line_3d (line_3d line, point_3d point);
inline line_equat calc_3d_dydx_line (point_3d p1, point_3d p2);


inline line_equat calc_3d_dydx_line (point_3d p1, point_3d p2)
{
  line_equat l;

  l.m = (p2.y - p1.y) / (p2.x - p1.x);
  l.b = p1.y - l.m * p1.x;
  
  return l;
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


inline point_3d offset_point_3d
  (point_3d point, double x, double y, double z)
{
  point_3d temp;
  
  temp.x = point.x + x;
  temp.y = point.y + y;
  temp.z = point.z + z;

  return temp;
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


inline y_line_3d parll_y_line_3d (y_line_3d line, point_3d point)
{
  y_line_3d temp;

  temp.mx = line.mx;
  temp.mz = line.mz;
  temp.bx = point.x - point.y * temp.mx;
  temp.bz = point.z - point.y * temp.mz;

  return temp;
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
          calc_line_3d (&z_line, point1, point2);
          double x = z_line.mx * point3.z + z_line.bx;
          double y = z_line.my * point3.z + z_line.by;
          plane->m2 = (point3.x - x) / (point3.y - y);
          plane->perp_m2 = -(point3.y - y) / (point3.x - x);
          plane->b = x - y * plane->m2;
        }
        else {
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
}


inline int point_relative_to_plane (plane_type plane, point_3d point)
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


inline int point_rel_to_plane (plane_type plane, point_3d point, double rad)
{
  if (plane.y_plane) {
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


inline x_line_3d calc_x_line_3d (point_3d p1, point_3d p2)
{
//  y = m1x + b1
//  z = m2x + b2
  x_line_3d line;
  
  line.my = (p2.y - p1.y) / (p2.x - p1.x);
  line.by = p1.y - line.my * p1.x;
  line.mz = (p2.z - p1.z) / (p2.x - p1.x);
  line.bz = p1.z - line.mz * p1.x;

  return line;
}


inline point_3d intersect_line_plane_3d (plane_type plane, line_3d line)
{
  point_3d p;

  p.z = (line.bx * plane.m1 + line.by * plane.m2 + plane.b) /
        (1 - plane.m1 * line.mx - plane.m2 * line.my);
  p.x = line.mx * p.z + line.bx;
  p.y = line.my * p.z + line.by;
  
  return p;
}


inline void calc_line_3d (line_3d* l3d, point_3d p1, point_3d p2)
{
  l3d->mx = (p2.x - p1.x) / (p2.z - p1.z);
  l3d->my = (p2.y - p1.y) / (p2.z - p1.z);
  l3d->bx = p1.x - l3d->mx * p1.z;
  l3d->by = p1.y - l3d->my * p1.z;
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


inline line_3d calc_line_3d (point_3d p1, point_3d p2)
{
  line_3d l3d;
  
  l3d.mx = (p2.x - p1.x) / (p2.z - p1.z);
  l3d.my = (p2.y - p1.y) / (p2.z - p1.z);
  l3d.bx = p1.x - l3d.mx * p1.z;
  l3d.by = p1.y - l3d.my * p1.z;

  return l3d;
}

#endif // !INCLUDE_MATH_3D
