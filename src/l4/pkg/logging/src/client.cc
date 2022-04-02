/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/sys/err.h>
#include <l4/sys/types.h>

#include <l4/cxx/iostream>
#include <l4/cxx/ipc_stream>

#include <thread>
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <random>

#include <l4/logging/shared.h>

int main() {
    L4::cout << "Client started\n";

    L4::Cap<Logger> server = L4Re::Env::env()->get_cap<Logger>("logger");
    if (!server.is_valid()) {
        L4::cout << "Could not get logger capability!\n";
        return 1;
    }
    
    std::srand(time(0));

    // make some randomly timed log messages (doesn't seem to be actually random though, maybe because both clients are initialized simultaneously and get the same time(0) for srand)
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000 + 500));
        if (server->print("Hello World!")) {
            L4::cout << "Error talking to server!\n";
        }
    }

    return 0;
}
