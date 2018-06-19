inline void show_cursor (point_3d pos, view_type view)
{
  point_2d p;
  p.x = 160;
  p.y = 100;
 
  if (_cursor_mode) {
    int r = 7;
    for (int x = -r; x <= r; x++) {
      pxl (p.x + x, p.y - r, 255, 0, 0);
      pxl (p.x + x, p.y + r, 255, 0, 0);
    }
    for (int y = -r; y <= r; y++) {
      pxl (p.x - r, p.y + y, 255, 0, 0);
      pxl (p.x + r, p.y + y, 255, 0, 0);
    }
  }
  else {
    for (int y = -5; y < 5; y++)
      pxl (p.x, p.y + y, 255, 0, 0);
    for (int x = -5; x < 5; x++)
      pxl (p.x + x, p.y, 255, 0, 0);
  }
}


inline void show_corner (point_type corner, view_type view)
{
  if (corner.rel.z >= view.z_cutoff) {
    point_2d p = map_to_scrn (view, corner.rel);
    int r = int(3 * (view.zoom.x / corner.rel.z));
    int x, y;
    
    if (r < 3)
      r = 3;
    else if (r > 35)
      r = 35;
      
    if (corner.select) {
      for (y = -r, x = -r; y < r; y++, x++)
        pxl (p.x + x, p.y + y, 0, 0, 255);
      for (y = -r, x = r; y < r; y++, x--)
        pxl (p.x + x, p.y + y, 0, 0, 255);
    }
    else {
      for (y = -r, x = -r; y < r; y++, x++)
        pxl (p.x + x, p.y + y, 255, 255, 255);
      for (y = -r, x = r; y < r; y++, x--)
        pxl (p.x + x, p.y + y, 255, 255, 255);
    }
  }
}


inline point_2d map_abs_point (point_3d p, view_type view)
{
  rotate (&p.x, &p.z, view.camera.pos.x, view.camera.pos.z,
    view.camera.angle_xz);
  rotate (&p.z, &p.y, view.camera.pos.z, view.camera.pos.y,
    view.camera.angle_yz);
  rotate (&p.x, &p.y, view.camera.pos.x, view.camera.pos.y,
    view.camera.angle_xy);
  
  p.x = p.x - view.camera.pos.x;
  p.y = p.y - view.camera.pos.y;
  p.z = p.z - view.camera.pos.z;

  return map_to_scrn (view, p);
}


inline void calc_view (camera_type cursor, view_type* view)
{
  if (_cursor_mode) {
    view->camera.angle_yz = cursor.angle_yz;
    view->camera.angle_xz = cursor.angle_xz;
    view->camera.pos = cursor.pos;
  }
  else {
    view->camera.angle_yz = cursor.angle_yz;
    view->camera.angle_xz = cursor.angle_xz;
    view->camera.pos = offset_point_3d (cursor.pos, 0, 0, -_view_rad);
    rotate (&view->camera.pos.z, &view->camera.pos.y,
            cursor.pos.z, cursor.pos.y, -cursor.angle_yz);
    rotate (&view->camera.pos.x, &view->camera.pos.z,
            cursor.pos.x, cursor.pos.z, -cursor.angle_xz);
  }
}


void cleanup_and_exit()
{
  key_delete();
  set_vesa_mode(0x3);
  exit(0);
}


inline void select_point (view_type view, point_type* start, point_type* last)
{
  for (point_type* p = start; p <= last; p++) {
    point_2d s = map_to_scrn (view, *p);
    if (s.x > 160 - 7)
      if (s.x < 160 + 7)
        if (s.y > 100 - 7)
          if (s.y < 100 + 7)
            p->select = !p->select;
  }
}


inline dlong curr_time()
{
  return (dlong)uclock() / ((dlong)UCLOCKS_PER_SEC / CLOCK_GRAN);
}


void create_tex_ref (
  tex_ref_type** last_ref, point_type** last_point,
  point_3d p1, point_3d p2, point_3d p3)
{
  plane_type plane;
  point_2d offset;
  offset.x = 10;
  offset.y = 15;
  
  calc_plane (&plane, p1, p2, p3);
  double ax = atan(plane.m1);
  double m2 = -1 / plane.m1;
  double b2 = p1.z - m2 * p1.x;
  x_line_3d ref = calc_x_line_3d (p2, p3);
  point_3d pb;
  pb.x = (b2 - ref.bz) / (ref.mz - m2);
  pb.y = ref.my * pb.x + ref.by;
  pb.z = ref.mz * pb.x + ref.bz;
  double ay = atan(edge_fall_m (pb, p1));
  _dtemp[10] = ax;
  _dtemp[11] = ay;

  point_3d temp[3];

  double w = 5, h = 5;
  temp[0].x = p1.x + offset.x;
  temp[0].y = p1.y + offset.y;
  temp[0].z = p1.z;
  temp[1].x = p1.x + w + offset.x;
  temp[1].y = p1.y + offset.y;
  temp[1].z = p1.z;
  temp[2].x = p1.x + offset.x;
  temp[2].y = p1.y + h + offset.y;
  temp[2].z = p1.z;
  
  rotate (&temp[0].z, &temp[0].y, p1.z, p1.y, ay);
  rotate (&temp[0].x, &temp[0].z, p1.x, p1.z, ax);
  rotate (&temp[1].z, &temp[1].y, p1.z, p1.y, ay);
  rotate (&temp[1].x, &temp[1].z, p1.x, p1.z, ax);
  rotate (&temp[2].z, &temp[2].y, p1.z, p1.y, ay);
  rotate (&temp[2].x, &temp[2].z, p1.x, p1.z, ax);
  
  (*last_point)++;
  (*last_point)->abs = temp[0];
  (*last_point)->select = false;
  
  (*last_point)++;
  (*last_point)->abs = temp[1];
  (*last_point)->select = false;

  (*last_point)++;
  (*last_point)->abs = temp[2];
  (*last_point)->select = false;
}


inline void create_tri
  (tri_type** last_tri, point_type* p1, point_type* p2, point_type* p3,
  view_type view)
{
  sort_clockwise (&p1, &p2, &p3, view);
  (*last_tri)++;
  tri_type* temp = *last_tri;

  temp->t3d.p1 = p1;
  temp->t3d.p2 = p2;
  temp->t3d.p3 = p3;
}


inline void sort_clockwise
  (point_type** p1, point_type** p2, point_type** p3, view_type view)
{
  point_2d s1;// = map_to_scrn(*p1, view);
  point_2d s2;// = map_abs_point(*p2, view);
  point_2d s3;// = map_abs_point(*p3, view);

  point_2d center;
  center.x = (s1.x + s2.x + s3.x) / 3;
  center.y = (s1.y + s2.y + s3.y) / 3;
  
  double a1 = -atan2(-center.y + s1.y, center.x - s1.x);
  double a2 = -atan2(-center.y + s2.y, center.x - s2.x);
  double a3 = -atan2(-center.y + s3.y, center.x - s3.x);

  if (a1 > a2)
    if (a3 < a2)
      ; // 3, 2, 1
    else if (a3 > a1) {
      swap_p3d (p1, p2); //  2, 1, 3
      swap_p3d (p1, p3);
    }
    else
      swap_p3d (p2, p3); // 2, 3, 1
  else
    if (a3 < a1)
      swap_p3d (p1, p2); // 3, 1, 2
    else if (a3 > a2)
      swap_p3d (p1, p3);// 1, 2, 3
    else {
      swap_p3d (p2, p3);// 1, 3, 2
      swap_p3d (p1, p3);
    }
}


inline void swap_p3d (point_type** p1, point_type** p2)
{
  point_type* temp = *p1;
  **p1 = **p2;
  **p2 = *temp;
}


inline void draw_tri_group (tri_group group, view_type view)
{
  if (point_relative_to_plane (group.plane, view.camera.pos) == group.vis_side)
    for (tri_type* tri = group.first; tri <= group.last; tri++)
      draw_tri
}