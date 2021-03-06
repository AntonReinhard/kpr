#include "paddle-server.h"
#include "l4/pong/logging.h"

#include <iostream>
#include <sstream>
#include <string>

static Paddle_server *the_paddle_server;

std::ostringstream os;

template< typename T >
T extract(L4::Ipc_istream &i)
{ T v; i >> v; return v; }


int Paddle_so::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios) {
    unsigned long op;
    ios >> op;
    switch (op) {
    case 1:
        move_to(extract<int>(ios));
        return L4_EOK;
    case 2:
        set_lifes(extract<int>(ios));
        return L4_EOK;
    case 3:
        ios << get_lifes();
        return L4_EOK;
    default:
        os << "Paddle_so: Invalid request for " << obj;
        send_ipc(os.str()); os.str("");
        return -L4_ENOREPLY;
    }
}

int Paddle_server::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios) {
    unsigned long op;
    ios >> op;
    switch (op) {
    case 1:
        ios << connect();
        return L4_EOK;
    default:
        os << "Paddle Server: Invalid request for " << obj;
        send_ipc(os.str()); os.str("");
        return -L4_ENOREPLY;
    }
}

Paddle_server::Paddle_server(Env &env, Paddle paddles[4])
: L4::PosixThread(),
    _pad0(&paddles[0]),
    _pad1(&paddles[1]),
    env(env) {
    //Paddle_so can't be copied anymore.
    //_pad[0] = Paddle_so(&paddles[0]);
    //_pad[1] = Paddle_so(&paddles[1]);
    _pad[0] = &_pad0;
    _pad[1] = &_pad1;

    _pad0.paddle()->add(0,60);
    _pad0.paddle()->add(16,60);
    _pad0.paddle()->add(18,55);
    _pad0.paddle()->add(20,50);
    _pad0.paddle()->add(21,40);
    _pad0.paddle()->add(21,20);
    _pad0.paddle()->add(20,10);
    _pad0.paddle()->add(18,5);
    _pad0.paddle()->add(16,0);
    _pad0.paddle()->add(0,0);

    _pad1.paddle()->add(20 ,60);
    _pad1.paddle()->add(20 , 0);
    _pad1.paddle()->add( 4 , 0);
    _pad1.paddle()->add( 2 , 5);
    _pad1.paddle()->add( 0 ,10);
    _pad1.paddle()->add(-1 ,20);
    _pad1.paddle()->add(-1 ,40);
    _pad1.paddle()->add( 0 ,50);
    _pad1.paddle()->add( 2 ,55);
    _pad1.paddle()->add( 4 ,60);
    send_ipc("------------------------------------------------------");
}

void Paddle_server::run() {
    send_ipc("Hello from svrloop");
    L4Re::Util::Registry_server<> server(l4_utcb(), self(),
                                        L4Re::Env::env()->factory());
    server.registry()->register_obj(this, "PongServer");
    server.registry()->register_obj(_pad[0]);
    server.registry()->register_obj(_pad[1]);
    the_paddle_server = this;
    //server._iostream = L4::Ipc_iostream(l4_utcb());
    server.loop();
}

void Paddle_server::handle_collision(Obstacle *o) {
    for (unsigned i = 0; i < 2; ++i) {
        if (_pad[i]->connected() && o==_pad[i]->paddle()->wall()) {
            _pad[i]->dec_lifes();
        }
    }
}

L4::Cap<void> Paddle_server::connect() {
    send_ipc("Connecting");
    for (unsigned i = 0; i < 2; ++i) {
        if (_pad[i]->connected()) {
            continue;
        }
        _pad[i]->connect(true);
        env.add(_pad[i]->paddle());

        os << "MAP cap: " << _pad[i]->obj_cap();
        send_ipc(os.str()); os.str("");

        return _pad[i]->obj_cap();
    }

    return L4::Cap<void>::Invalid;
}

void Paddle_so::move_to(int pos) {
    if (pos<0 || pos>1023) {
        return;
    }

    paddle()->move(0,pos*420/1024);
}


int *Paddle_so::lifes() {
    return &_points;
}

void Paddle_so::set_lifes(int _lifes) {
    int *l = lifes();
    if (l) {
        *l = _lifes;
    }
}

int Paddle_so::get_lifes() {
    int *l = lifes();
    if (l) {
        return *l;
    }

    return -5000;
}
