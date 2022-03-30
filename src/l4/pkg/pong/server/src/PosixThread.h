/*
 * (c) 2012 Nils Asmussen <nils@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#pragma once

#include <l4/re/error_helper>
#include <pthread.h>
#include <pthread-l4.h>

#include "l4/pong/logging.h"

namespace L4 {

class PosixThread {
public:
	PosixThread()
		: _thread() {
	};
	virtual ~PosixThread() {
	};

	L4::Cap<L4::Thread> self() const {
		return L4::Cap<L4::Thread>(pthread_l4_cap(pthread_self()));
	};

	void start() {
		L4Re::chksys(pthread_create(&_thread,NULL,&PosixThread::start_run,this));
	};

private:
	virtual void run() = 0;

	PosixThread(const PosixThread&);
	PosixThread& operator=(const PosixThread&);

	static void* start_run(void* arg) {
		static_cast<PosixThread*>(arg)->run();
		return NULL;
	};

private:
	pthread_t _thread;
};

}
