#ifndef PADDLE_THREAD_H__
#define PADDLE_THREAD_H__

#include "PosixThread.h"
#include "env.h"
#include "paddle.h"
#include "obj_reg.h"

#include <l4/cxx/ipc_server>
#include <l4/sys/capability>


class Paddle_so : public L4::Server_object
{
public:
  /*explicit*/ Paddle_so(Paddle *_pad = 0)
  : _points(0), _py(0), _paddle(_pad), _connected(0) {}
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);

  void move_to(int pos);
  void set_lifes(int lifes);
  int get_lifes();

  Paddle *paddle() const { return _paddle; }

  bool connected() const { return _connected; }
  void connect(bool c = true) { _connected = c; }

  void dec_lifes() { --_points; }

  // This used to be defined in L4::Server_object
  Demand get_buffer_demand() const override { return Demand(0); }

private:
  int *lifes();

  int _points;
  int _py;
  Paddle *_paddle;
  bool _connected;
};


class Paddle_server : public L4::PosixThread, public L4::Server_object
{
public:

  Paddle_server(Env &env, Paddle paddles[4]);

  void run();

  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);

  L4::Cap<void> connect();

  void handle_collision( Obstacle *o );

  // This used to be defined in L4::Server_object
  Demand get_buffer_demand() const override { return Demand(0); }

private:

  char str[60];

  Paddle_so _pad0;
  Paddle_so _pad1;
  // _pad[i] == &_padi for i in {0,1}.  Had to move the actual objects
  // out of the array because Paddle_so can't be moved or copied, and
  // thus I can't construct the array nicely
  Paddle_so *_pad[2];

  Env &env;
  char stack[4096];
};


#endif //PADDLE_THREAD_H__
