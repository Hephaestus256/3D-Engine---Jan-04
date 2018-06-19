#include <go32.h>
#include <dpmi.h>
#include <dos.h>

#ifndef INCLUDE_CONTROL
#define INCLUDE_CONTROL

struct key_type {
  bool press;
  bool change;
};

key_type _keyb_stat[128];
key_type _mouse_btn[2];
_go32_dpmi_seginfo _old_key_handler, _new_key_handler;

inline void mouse_button (int raw_dat, int n);
void key_handler();
void log_key(int k);
void key_init();
void key_delete();
int init_mouse();
void show_cursor();
void hide_cursor();
void set_mouse_bound (int x1, int y1, int x2, int y2);
void get_mouse_status (short* x, short* y, short* b);
void set_mouse_coord (int x, int y);
inline double units_to_rad (double units, double units_per_rot);


inline void mouse_button (int raw_dat, int n)
{
  if (raw_dat & n) {
    if (!_mouse_btn[n - 1].press)
      _mouse_btn[n - 1].change = true;
    else
      _mouse_btn[n - 1].change = false;
    _mouse_btn[n - 1].press = true;
  }      
  else {
    _mouse_btn[n - 1].change = false;
    _mouse_btn[n - 1].press = false;
  }
}


void key_handler()
{
  unsigned char al, ah;
  int raw_key;
  
  asm("cli; pusha");
  raw_key = inportb(0x60);
  al = inportb(0x61);
  al |= 0x82;
  outportb(0x61, al);
  al &= 0x7f;
  outportb(0x61, al);
  log_key(raw_key);
  outportb(0x20, 0x20);
  asm("popa; sti");
}


void log_key(int k)
{
  if (k < 128) {
    if (!_keyb_stat[k & 127].press)
      _keyb_stat[k & 127].change = true;
    _keyb_stat[k & 127].press = true;
  }
  else
    _keyb_stat[k & 127].press = false;
}


void key_init() // Install new key handler
{
  _new_key_handler.pm_offset = (int)key_handler;
  _new_key_handler.pm_selector = _go32_my_cs();
  _go32_dpmi_get_protected_mode_interrupt_vector(0x9, &_old_key_handler);
  _go32_dpmi_allocate_iret_wrapper(&_new_key_handler);
  _go32_dpmi_set_protected_mode_interrupt_vector(0x9, &_new_key_handler);
}


void key_delete() // Put standard key handler back
{
  _go32_dpmi_set_protected_mode_interrupt_vector(0x9, &_old_key_handler);
}


int init_mouse()
{
  union REGS r;
  
  r.x.ax = 0x0;
  int86(0x33, &r, &r);

  if (r.x.ax)
    return true;
  else
    return false;
}


void show_cursor()
{
  union REGS r;
  
  r.x.ax = 0x1;
  int86(0x33, &r, &r);
}


void hide_cursor()
{
  union REGS r;
  
  r.x.ax = 0x2;
  int86(0x33, &r, &r);
}


void set_mouse_bound (int x1, int y1, int x2, int y2)
{
  union REGS r;
  
  r.x.ax = 0x7;
  r.x.cx = short(x1);
  r.x.dx = short(x2);
  int86(0x33, &r, &r);

  r.x.ax = 0x8;
  r.x.cx = short(y1);
  r.x.dx = short(y2);
  int86(0x33, &r, &r);
}


void get_mouse_status (short* x, short* y, short* b)
{
  union REGS r;
 
  r.x.ax = 0x3;
  int86(0x33, &r, &r);
  *x = r.x.cx;// >> (signed)3;
  *y = r.x.dx;// >> (signed)3;
  *b = r.x.bx;
}


void set_mouse_coord (int x, int y)
{
  union REGS r;

  r.x.ax = 0x4;
  r.x.cx = x;
  r.x.dx = y;
  int86(0x33, &r, &r);
}


inline double units_to_rad (double units, double units_per_rot)
{
  return (units * PI * 2) / units_per_rot;
}


#endif // !INCLUDE_CONTROL
