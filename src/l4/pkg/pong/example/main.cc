#include <l4/util/util.h>
#include <l4/re/env>
#include <l4/re/namespace>

#include <l4/cxx/exceptions>
#include <l4/cxx/ipc_stream>

#include <l4/re/util/cap_alloc>

#include <l4/pong/logging.h>

#include <l4/keyboard_driver/shared.h>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <pthread-l4.h>

void *thread_fn(void*);

class Paddle {
public:
    Paddle(int speed, unsigned long svr);
    void run();

    int connect();
    int lives();
    void move(int pos);

private:
    unsigned long svr;
    unsigned long pad_cap;
    int speed;
};

class Main {
public:
    void run();
};

int Paddle::connect() {
    L4::Ipc::Iostream s(l4_utcb());
    pad_cap = L4Re::Util::cap_alloc.alloc<void>().cap();
    while (1) {  
        std::ostringstream os;
        os << "PC: connect to 0x" << std::hex << svr;
        send_ipc(os.str().c_str()); //os.str("");
        s << 1UL;
        s << L4::Small_buf(pad_cap);
        send_ipc("ha ha ha ha");
        l4_msgtag_t err = s.call(svr, 1);
        send_ipc("staying alive, staying alive");
        l4_umword_t fp;
        s >> fp;

        os << "FP: " << fp <<  " err=" << err.raw;
        send_ipc(os.str().c_str()); os.str("");

        if (!l4_msgtag_has_error(err) && fp != 0) {
            os << "Connected to paddle " << (unsigned)fp;
            send_ipc(os.str().c_str()); os.str("");
            return pad_cap;
        }
        else {
            switch (l4_utcb_tcr()->error) {
            case L4_IPC_ENOT_EXISTENT:
                send_ipc("No paddle server found, retry");
                l4_sleep(1000);
                s.reset();
                break;
            default:
                os << "Connect to paddle failed err=0x" << std::hex << l4_utcb_tcr()->error;
                send_ipc(os.str().c_str()); os.str("");
                return l4_utcb_tcr()->error;
            };
        }
    }
    return 0;
}

int Paddle::lives() {
    L4::Ipc::Iostream s(l4_utcb());
    s << 3UL;
    if (!l4_msgtag_has_error((s.call(pad_cap)))) {
        int l;
        s >> l;
        return l;
    }

    return -1;
}

void Paddle::move(int pos) {
    L4::Ipc::Iostream s(l4_utcb());
    s << 1UL << pos;
    s.call(pad_cap);
    l4_sleep(10);
}

Paddle::Paddle(int speed, unsigned long svr)
    : svr(svr)
    , speed(speed) {}

void Paddle::run() {
    send_ipc("Pong client running...");
    int paddle = connect();
    if (paddle == -1) {
        return;
    }

    int pos = 180;
    std::ostringstream os;

    int c = 0;
    while(1) {
        if (c++ >= 500) {
            c = 0;
            os << '(' << pthread_self() << ") Lives: " << lives();
            send_ipc(os.str().c_str()); os.str("");
        }
        move(pos);
        pos += speed;
        if (pos < 0) {
            pos = 0;
            speed = -speed;
        }
        if (pos > 1023) {
            pos = 1023;
            speed = -speed;
        }
    }
}

static l4_cap_idx_t server() {
    L4::Cap<void> s = L4Re::Env::env()->get_cap<void>("PongServer");
    if (!s) {
        throw L4::Element_not_found();
    }

    return s.cap();
}

Paddle p0(-10, server());
Paddle p1(20, server());

void *thread_fn(void* ptr) {
    Paddle *pd = (Paddle*)ptr;
    pd->run();
    return 0;
}

void Main::run() {
    l4_sleep(3000);
    send_ipc("Hello from pong example client");

    pthread_t p, q;

    pthread_create(&p, NULL, thread_fn, (void*)&p0);
    pthread_create(&q, NULL, thread_fn, (void*)&p1);

    send_ipc("PC: main sleep...");
    l4_sleep_forever();
}

int main() {
    /** doesn't work anyways...
    redirect_to_log(std::cout);
    redirect_to_log(std::cerr);
    */

    Main().run();
    return 0;
};
