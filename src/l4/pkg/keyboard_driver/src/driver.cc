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
#include <l4/re/util/br_manager>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/sys/cxx/ipc_epiface>
#include <pthread-l4.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <cstdlib>
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
#include <l4/keyboard_driver/shared.h>


char buf[64];
constexpr unsigned bufsize = 63;

struct keys {
    bool up = false;
    bool down = false;
    bool w = false;
    bool s = false;
};

// function receiving IRQs forever
int recv();

// function looping the session server forever
void* loopServer(void*);

keys buttons_pressed;

class KeyboardServer : public L4::Epiface_t<KeyboardServer, KeyboardDriver> {
public:
    KeyboardServer(const L4::String& id)
        : id(strdup(id.p_str())) {
        L4::cout << "Keyboard Client with ID " << id << " created\n";
    }

    ~KeyboardServer() {
        delete id.p_str();
    }

    unsigned op_getkeys(KeyboardDriver::Rights) {
        unsigned ret = 0;
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
        L4::cout << "Create called\n";
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

        auto* kbserver = new KeyboardServer(tag);

        if (kbserver == nullptr) {
            L4::cerr << "Could not create keyboard server, memory allocation failed\n";
            return -L4_ENOMEM;
        }

        //server.registry()->register_obj(kbserver);
        res = L4::Ipc::make_cap_rw(kbserver->obj_cap());

        return L4_EOK;
    }
};

int recv() {
    // most of this setup is taken from the example implementation
    int irqno = 1;
    l4_cap_idx_t irqcap, icucap;
    l4_msgtag_t tag;
    int err;
    icucap = l4re_env_get_cap("icu");

    /* Get a free capability slot for the ICU capability */
    if (l4_is_invalid_cap(icucap)) {
        L4::cerr << "Did not find an ICU\n";
        return 1;
    }

    /* Get another free capaiblity slot for the corresponding IRQ object*/
    if (l4_is_invalid_cap(irqcap = l4re_util_cap_alloc())) {
        return 1;
    }
    /* Create IRQ object */
    if (l4_error(tag = l4_factory_create_irq(l4re_global_env->factory, irqcap))) {
        snprintf(buf, bufsize, "Could not create IRQ object: %lx\n", l4_error(tag));
        L4::cerr << buf;
        return 1;
    }

    /*
    * Bind the recently allocated IRQ object to the IRQ number irqno
    * as provided by the ICU.
    */
    if (l4_error(l4_icu_bind(icucap, irqno, irqcap))) {
        snprintf(buf, bufsize, "Binding IRQ%d to the ICU failed\n", irqno);
        L4::cerr << buf;
        return 1;
    }

    /* Bind ourselves to the IRQ */
    tag = l4_rcv_ep_bind_thread(irqcap, l4re_env()->main_thread, 0xDEAD);
    if ((err = l4_error(tag))) {
        snprintf(buf, bufsize, "Error binding to IRQ %d: %d\n", irqno, err);
        L4::cerr << buf;
        return 1;
    }

    snprintf(buf, bufsize, "Attached to key IRQ %d\n", irqno);
    L4::cout << buf;

    if (l4io_request_ioport(0x60, 1)) {
        L4::cerr << "Failed to request ioport\n";
    }

    /* IRQ receive loop */
    while (1) {
        /* Wait for the interrupt to happen */
        tag = l4_irq_receive(irqcap, L4_IPC_NEVER);
        if ((err = l4_ipc_error(tag, l4_utcb()))) {
            snprintf(buf, bufsize, "Error on IRQ receive: %d\n", err);
            L4::cerr << buf;
        }
        else {
            /* Process the interrupt -- may do a 'break' */
            auto scancode = l4util_in8(0x60);
            bool pressed = !(0x80 & scancode);  // was the button pressed or released?
            scancode = ~0x80 & scancode;        // what was the scancode of the key

            switch (scancode) {
            case 72:    // up arrow
                buttons_pressed.up = pressed; break;
            case 80:    // down arrow
                buttons_pressed.down = pressed; break;
            case 17:    // w
                buttons_pressed.w = pressed; break;
            case 31:    // s
                buttons_pressed.s = pressed; break;
            default:    // any other key, ignore
                break;
            }
        }
    }

    /* We're done, detach from the interrupt. */
    tag = l4_irq_detach(irqcap);
    if ((err = l4_error(tag))) {
        snprintf(buf, bufsize, "Error detach from IRQ: %d\n", err);
        L4::cerr << buf;
    }

    return 0;
}

void* loopServer(void*) {
    L4::cout << "starting keyboard server\n";
    // make registry server that knows what thread it lives in
    KeyboardServer kbserver("KeyboardServer");
    SessionServer sserver;
    L4Re::Util::Registry_server<> server(l4_utcb(), L4::Cap<L4::Thread>(pthread_l4_cap(pthread_self())), L4::Cap<L4::Factory>(sserver.obj_cap()));

    // Register session server
    if (!server.registry()->register_obj(&kbserver, "kbdrv").is_valid()) {
        L4::cerr << "Could not register my service (session server), is there a 'kbdrv' in the caps table?\n";
        return nullptr;
    }

    server.loop();
    return nullptr;
}

int main() {

    pthread_t serverLoop;
    pthread_create(&serverLoop, nullptr, &loopServer, nullptr);

    // receive irq
    recv();

    return 0;
}
