// -*- C++ -*-

#pragma once

#include <ostream>
#include <string.h>
#include <mutex>

#include <l4/logging/shared.h>

void init_logger(L4::Cap<Logger>& logger);
void send_ipc(std::string const& msg);
void redirect_to_log(std::ostream& os);

// needs to be inline because the definition is in a header and otherwise it won't link if it tries to export the symbols
inline void init_logger(L4::Cap<Logger>& logger) {
    // get logger cap
    logger = L4Re::Env::env()->get_cap<Logger>("logger");
    if (!logger.is_valid()) {
        printf("Could not get logger capability!\n");
    }
}

inline void send_ipc(std::string const& msg) {
    static L4::Cap<Logger> logger;
    static bool initialized = false;
    if (!initialized) {
        init_logger(logger);
        initialized = true;
    }
    logger->print(msg.c_str());
}

template <typename CharT = char>
class Log_Buffer : public std::basic_streambuf<CharT>
{
private:
        std::string buffer;
public:

    using Base = std::basic_streambuf<CharT>;
    using char_type = typename Base::char_type;
    using int_type = typename Base::int_type;

    Log_Buffer(std::size_t size) : buffer(size, '\0') {
        Base::setp(&buffer.front(), &buffer.back());
    }

    int_type overflow(int_type ch) override {
        Log_Buffer::sync();
        Base::sputc(ch);
        return ch;
    }

    int sync() override {
        static std::mutex mtx;
        std::unique_lock<std::mutex> lck(mtx);
        // would also have to lock the operator<< but there are a lot of overloads...
        // also this doesn't handle newlines, and because the clients never use flush or endl this only gets called when the buffer overflows
        // which isn't very helpful to get logs
        send_ipc(buffer);
        memset(&buffer[0], '\0', 256);
        Base::setp(this->pbase(), this->epptr());
        return 0;
    }
};

inline void redirect_to_log(std::ostream& os)
{
    // The basic_ios interface expectes a raw pointer...
    auto buf = new Log_Buffer<char>(256);
    auto old_buf = os.rdbuf(buf);
    // I guess we should delete this here?
    delete old_buf;
}
