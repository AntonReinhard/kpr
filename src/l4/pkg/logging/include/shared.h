/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#pragma once

#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_iface>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/re/util/br_manager>

#include <l4/cxx/ipc_stream>
#include <l4/cxx/iostream>

struct Logger : L4::Kobject_t<Logger, L4::Kobject, 0x44> {
  L4_INLINE_RPC(int, print, (L4::Ipc::String<>));
  L4_INLINE_RPC(int, create, (L4::Ipc::Cap<void>&, L4::Ipc::Varg_list_ref));
  typedef L4::Typeid::Rpcs<print_t> Rpcs;
};
