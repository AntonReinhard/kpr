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

// return codes for pressed buttons as masks
constexpr int KB_NONE = 0;
constexpr int KB_UP_1 = 1;
constexpr int KB_UP_2 = 2;
constexpr int KB_DOWN_1 = 4;
constexpr int KB_DOWN_2 = 8;

struct KeyboardDriver : L4::Kobject_t<KeyboardDriver, L4::Kobject, 0x44> {
  L4_INLINE_RPC(int, getkeys, (void));
  L4_INLINE_RPC(int, create, (L4::Ipc::Cap<void>&, L4::Ipc::Varg_list_ref));
  typedef L4::Typeid::Rpcs<getkeys_t> Rpcs;
};
