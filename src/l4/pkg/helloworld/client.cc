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

#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "shared.h"

int main()
{
    L4::Cap<Helloworld> server = L4Re::Env::env()->get_cap<Helloworld>("hw_channel");
    if (!server.is_valid())
    {
        L4::cout << "Could not get server capability!\n";
        return 1;
    }

    sleep(2);

    while (true)
    {
        L4::cout << "Sending string to helloworld_server\n";
        if (server->print("Hello World!"))
        {
            L4::cout << "Error talking to server!\n";
        }
        sleep(1);
    }

    return 0;
}
