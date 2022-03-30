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
#include <cstdlib>
#include <thread>
#include <tuple>
#include <string>
#include <mutex>
#include <chrono>

#include <l4/re/c/util/cap_alloc.h>
#include <l4/cxx/iostream>
#include <l4/cxx/ipc_stream>
#include <l4/cxx/string.h>
#include <l4/cxx/string>

#include <l4/io/io.h>
#include <l4/util/port_io.h>

#include <l4/logging/shared.h>

static L4Re::Util::Registry_server<> server;
static L4::Cap<Logger> logger;

char buf[64];
unsigned bufsize = 63;

struct keys {
    bool up = false;
    bool down = false;
    bool w = false;
    bool s = false;
};

keys buttons_pressed;

class KeyboardServer : public L4::Epiface_t<KeyboardServer, KeyboardDriver> {
public:
    KeyboardServer(const L4::String& id)
        : id(strdup(id.p_str())) {
        std::string text = "Keyboard Client with ID " + std::string(id.p_str(), id.length()) + " created";
        logger->print(L4::Ipc::String<>(text.c_str()));
    }

    ~KeyboardServer() {
        delete id.p_str();
    }

    int op_getkeys(KeyboardDriver::Rights) {
        logger->print("Received getkeys request");
        int ret = 0;
        if (buttons_pressed.up) {
            ret |= KB_UP_1;
        }
        if (buttons_pressed.down) {
            ret |= KB_DOWN_1;
        }
        if (buttons_pressed.w) {
            ret |= KB_UP_2;
        }        
        if (buttons_pressed.s) {
            ret |= KB_DOWN_2;
        }
        return ret;
    }

private:
    L4::String id;
};

class SessionServer : public L4::Epiface_t<SessionServer, L4::Factory> {
public:
    int op_create(L4::Factory::Rights, L4::Ipc::Cap<void>& res, l4_mword_t type, L4::Ipc::Varg_list_ref args) {
        if (type != 0) {
            return -L4_ENODEV;
        }

        L4::String tag;
        for (L4::Ipc::Varg arg : args) {
            if (arg.is_of<char const*>()) {
                tag = arg.value<char const*>();
                break;
            }
        }

        auto* logserver = new KeyboardServer(tag);

        if (logserver == nullptr) {
            logger->print("Could not create keyboard server, memory allocation failed");
            return -L4_ENOMEM;
        }

        server.registry()->register_obj(logserver);
        res = L4::Ipc::make_cap_rw(logserver->obj_cap());

        logger->print("New keyboard server created");
        return L4_EOK;
    }
};

int recv() {
    int irqno = 1;
    l4_cap_idx_t irqcap, icucap;
    l4_msgtag_t tag;
    int err;
    icucap = l4re_env_get_cap("icu");

    /* Get a free capability slot for the ICU capability */
    if (l4_is_invalid_cap(icucap)) {
        snprintf(buf, bufsize, "Did not find an ICU");
        logger->print(buf);
        return 1;
    }

    /* Get another free capaiblity slot for the corresponding IRQ object*/
    if (l4_is_invalid_cap(irqcap = l4re_util_cap_alloc())) {
        return 1;
    }
    /* Create IRQ object */
    if (l4_error(tag = l4_factory_create_irq(l4re_global_env->factory, irqcap))) {
        snprintf(buf, bufsize, "Could not create IRQ object: %lx", l4_error(tag));
        logger->print(buf);
        return 1;
    }

    /*
    * Bind the recently allocated IRQ object to the IRQ number irqno
    * as provided by the ICU.
    */
    if (l4_error(l4_icu_bind(icucap, irqno, irqcap))) {
        snprintf(buf, bufsize, "Binding IRQ%d to the ICU failed", irqno);
        logger->print(buf);
        return 1;
    }

    /* Bind ourselves to the IRQ */
    tag = l4_rcv_ep_bind_thread(irqcap, l4re_env()->main_thread, 0xDEAD);
    if ((err = l4_error(tag))) {
        snprintf(buf, bufsize, "Error binding to IRQ %d: %d", irqno, err);
        logger->print(buf);
        return 1;
    }

    snprintf(buf, bufsize, "Attached to key IRQ %d", irqno);
    logger->print(buf);

    if (l4io_request_ioport(0x60, 1)) {
        logger->print("Failed to request ioport");
    }

    logger->print("Keyboard setup complete");

    /* IRQ receive loop */
    while (1) {
        int label = 0;
        /* Wait for the interrupt to happen */
        tag = l4_irq_receive(irqcap, L4_IPC_NEVER);
        if ((err = l4_ipc_error(tag, l4_utcb()))) {
            snprintf(buf, bufsize, "Error on IRQ receive: %d", err);
            logger->print(buf);
        }
        else {
            /* Process the interrupt -- may do a 'break' */
            auto scancode = l4util_in8(0x60);
            bool pressed = 0x80 & scancode;
            scancode = ~0x80 & scancode;

            switch (scancode) {
            case 72: 
                buttons_pressed.up = pressed; break;
            case 80: 
                buttons_pressed.down = pressed; break;
            case 17: 
                buttons_pressed.w = pressed; break;
            case 31: 
                buttons_pressed.s = pressed; break;
            default: // unknown key, ignore
                /*
                if (pressed) {
                    snprintf(buf, bufsize, "Pressed key %u", scancode);
                }
                else {
                    snprintf(buf, bufsize, "Released key %u", scancode);
                }
                logger->print(buf);
                */
                break;
            }
        }
    }

    /* We're done, detach from the interrupt. */
    tag = l4_irq_detach(irqcap);
    if ((err = l4_error(tag))) {
        snprintf(buf, bufsize, "Error detach from IRQ: %d", err);
        logger->print(buf);
    }

    return 0;
}

int main() {
    static SessionServer sserver;

    // get logger cap
    logger = L4Re::Env::env()->get_cap<Logger>("logger");
    if (!logger.is_valid()) {
        L4::cout << "Could not get logger capability!\n";
        return 1;
    }

    // Register session server
    if (!server.registry()->register_obj(&sserver, "kbdrv").is_valid()) {
        logger->print("Could not register my service (session server), is there a 'kbdrv' in the caps table?");
        return 1;
    }

    sleep(1);

    std::thread t([](){
        // Wait for client requests
        server.loop();
    });

    // receive irq (has to be in main thread)
    recv();

    if (t.joinable()) {
        t.join();
    }

    return 0;
}
