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

#include <l4/re/env>
#include <l4/re/util/br_manager>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/sys/cxx/ipc_epiface>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#include <l4/cxx/iostream>
#include <l4/cxx/ipc_stream>
#include <l4/cxx/string.h>
#include <l4/cxx/string>

static L4Re::Util::Registry_server<> server;

class LoggingServer : public L4::Epiface_t<LoggingServer, Logger> {
public:
    LoggingServer(const L4::String& id)
        : id(strdup(id.p_str())) {
        L4::cout << "Logging Server ID: " << this->id << "\n";
        L4::cout << "Address: " << this << "\n";
    }

    ~LoggingServer() {
        delete id.p_str();
    }

    int op_print(Logger::Rights, L4::Ipc::String<> s) {
        L4::cout << "[" << this->id;
        L4::cout << "]: " << s.data << "\n";
        return 0;
    }

private:
    L4::String id;
};

class SessionServer : public L4::Epiface_t<SessionServer, L4::Factory> {
public:
    int op_create(L4::Factory::Rights, L4::Ipc::Cap<void>& res, l4_mword_t type, L4::Ipc::Varg_list_ref args) {
        L4::cout << "create call\n";
        if (type != 0) {
            return -L4_ENODEV;
        }

        int i = 0;
        L4::String tag;
        for (L4::Ipc::Varg arg : args) {
            if (arg.is_of<char const*>()) {
                L4::cout << "Argument [" << i << "]: " << arg.value<char const*>() << "\n";
                tag = arg.value<char const*>();
                break;
            }
        }

        auto* logserver = new LoggingServer(tag);

        if (logserver == nullptr) {
            L4::cout << "Could not create logserver, memory allocation failed\n";
            return -L4_ENOMEM;
        }

        server.registry()->register_obj(logserver);
        res = L4::Ipc::make_cap_rw(logserver->obj_cap());

        L4::cout << "New logging server created\n";
        return L4_EOK;
    }
};

int main() {
    static SessionServer sserver;

    for (int i = 0; i < 1000; ++i) {
        auto a = new std::vector<int>(i * 10);
        delete a;
    }

    // Register session server
    if (!server.registry()->register_obj(&sserver, "logger").is_valid()) {
        L4::cout << "Could not register my service (session server), is there a 'logger' in the caps table?\n";
        return 1;
    }

    L4::cout << "Welcome to the Logging server!\n";
    L4::cout << "I print messages sent to me.\n";

    // Wait for client requests
    server.loop();

    L4::cout << "Exiting\n";

    return 0;
}
