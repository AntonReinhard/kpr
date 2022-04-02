#include "gfx-drv.h"
#include <l4/sys/kdebug.h>
#include <l4/cxx/exceptions>
#include <l4/re/video/goos>
#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/re/error_helper>

#include <typeinfo>
#include <cstring>
#include <iostream>
#include <sstream>

#include "l4/pong/logging.h"

Screen::Screen()
{
  std::ostringstream os;
  L4::Cap<L4Re::Video::Goos> video;
  try
    {
      video = L4Re::Env::env()->get_cap<L4Re::Video::Goos>("vesa");
      if (!video)
	      throw L4::Element_not_found();
    }
  catch(L4::Base_exception const &e)
    {
      os << "Error looking for vesa device: " << e.str();
      send_ipc(os.str()); os.str("");
      return;
    }
  catch(...)
    {
      send_ipc("Caught unknown exception");
      return;
    }


  if (!video.is_valid())
    {
      send_ipc("No video device found");
      return;
    }

  send_ipc("constructed a valid screen");

  L4Re::Video::Goos::Info gi;
  video->info(&gi);

  L4::Cap<L4Re::Dataspace> fb = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  video->get_static_buffer(0, fb);

  printf("and screen is valid\n");

  if (!fb.is_valid()) {
    send_ipc("Invalid frame buffer object");
    return;
  }


  void *fb_addr = (void*)0x10000000;
  L4Re::chksys(L4Re::Env::env()->rm()->attach(&fb_addr, fb->size(), L4Re::Rm::F::Search_addr, fb, 0));

  if (!fb_addr){
    send_ipc("Cannot map frame buffer");
    return;
  }

  L4Re::Video::View v = video->view(0);
  L4Re::Video::View::Info vi;
  L4Re::chksys(v.info(&vi));

  _base        = (unsigned long)fb_addr + vi.buffer_offset;
  _line_bytes  = vi.bytes_per_line;
  _width       = vi.width;
  _height      = vi.height;
  _bpp	       = vi.pixel_info.bytes_per_pixel();

  if (   !vi.pixel_info.r().size()
      || !vi.pixel_info.g().size()
      || !vi.pixel_info.b().size())
    {
      send_ipc("Something is wrong with the color mapping");
      send_ipc("assume rgb 5:6:5");
      _red_shift   = 11; _red_size   = 5;
      _green_shift = 5;  _green_size = 6;
      _blue_shift  = 0;  _blue_size  = 5;
    }
  else
    {
      _red_shift   = vi.pixel_info.r().shift();
      _green_shift = vi.pixel_info.g().shift();
      _blue_shift  = vi.pixel_info.b().shift();
      _red_size    = vi.pixel_info.r().size();
      _green_size  = vi.pixel_info.g().size();
      _blue_size   = vi.pixel_info.b().size();
    }

  os << "Framebuffer: base   = 0x" << std::hex << _base; send_ipc(os.str()); os.str("");
  os << "             width  = " << std::dec << (unsigned)_width; send_ipc(os.str()); os.str("");
  os << "             height = " << (unsigned)_height; send_ipc(os.str()); os.str("");
	os << "             bpl    = " << _line_bytes; send_ipc(os.str()); os.str("");
	os << "             bpp    = " << _bpp; send_ipc(os.str()); os.str("");
	os << "             mode   = ("
	   << _red_size << ':' << _green_size << ':' << _blue_size << ")"; send_ipc(os.str()); os.str("");
  printf("end of constructor\n");
}
