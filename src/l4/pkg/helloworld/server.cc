/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "shared.h"

#include <stdio.h>
#include <unistd.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/re/util/br_manager>
#include <l4/sys/cxx/ipc_epiface>

#include <l4/cxx/iostream>
#include <l4/cxx/ipc_stream>

static L4Re::Util::Registry_server<L4Re::Util::Br_manager_hooks> server;

class Helloworld_server
    : public L4::Epiface_t<Helloworld_server, Helloworld> {
public:

  int op_print(Helloworld::Rights, L4::Ipc::String<> s) {
    L4::cout << "Received string of length: " << s.length << "\n";
//    L4::cout << s; // can't figure out how to actually print this string because there is *no* documentation
    L4::cout << "Hello World!\n";
    return 0;
  }

};


int main() {
  static Helloworld_server helloworld;

  // Register server
  if (!server.registry()->register_obj(&helloworld, "hw_channel").is_valid()) {
      L4::cout << "Could not register my service, is there a 'hw_channel' in the caps table?\n";
      return 1;
  }

  L4::cout << "Welcome to the hello world server!\n";
  L4::cout << "I print messages sent to me.\n";

  // Wait for client requests
  server.loop();

  return 0;
}
