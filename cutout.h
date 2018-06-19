class tri_cut {
  inline void disect_tri_b (point_2d p1, point_2d p2, point_2d p3);
  inline void disect_tri_c (point_2d p1, point_2d p2, point_2d p3);
  inline void disect_tri_d (point_2d p1, point_2d p2, point_2d p3);
  inline void store_inside (point_2d p1, point_2d p2, point_2d p3);
  inline void store_outside (point_2d p1, point_2d p2, point_2d p3);
public:
  inline void disect_tri
    (point_2d p1, point_2d p2, point_2d p3, tri_2d* i, tri_2d* o);
  tri_2d* inside;
  tri_2d* outside;
  line_equat* line;
  bool edge_exist[4];
  point_2d qa[4], qb[4];
};


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


inline void tri_cut::disect_tri
  (point_2d p1, point_2d p2, point_2d p3, tri_2d* i, tri_2d* o)
{
  inside = i; outside = o;
  line_equat line_12, line_23, line_31;

  calc_2d_dydx_line (&line_12, p1, p2);
  calc_2d_dydx_line (&line_23, p2, p3);
  calc_2d_dydx_line (&line_31, p3, p1);

  int color1 = 4, color2 = 9;

  if (!edge_exist[0]) {
    disect_tri_b (p1, p2, p3);
    return;
  }
  
  if (approx_equal (qa[0].x, qb[0].x))
    if (qa[0].y < qb[0].y)
      if (p1.x < qa[0].x)
        if (p2.x < qa[0].x)
          if (p3.x < qa[0].x)
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
