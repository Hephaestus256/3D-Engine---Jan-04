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


inline double calc_b_offset (double rad, double dx, double dy)
{
  return fabs(rad / sin(atan2(dy, dx)));
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


