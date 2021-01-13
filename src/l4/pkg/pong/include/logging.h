// -*- C++ -*-

#pragma once

#include <ostream>

void send_ipc(std::string const& msg);
void redirect_to_log(std::ostream& os);

void send_ipc(std::string const& msg)
{
        printf("%s", msg.data());
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

        Log_Buffer(std::size_t size) : buffer(size, '\0')
        {
                Base::setp(&buffer.front(), &buffer.back());
        }

        int_type overflow(int_type ch) override
        {
                Log_Buffer::sync();
                Base::sputc(ch);
                return ch;
        }

        int sync() override
        {
                send_ipc(buffer);
                Base::setp(this->pbase(), this->epptr());
                return 0;
        }
};

void redirect_to_log(std::ostream& os)
{
        // The basic_ios interface expectes a raw pointer...
        auto buf = new Log_Buffer<char>(256);
        auto old_buf = os.rdbuf(buf);
        // I guess we should delete this here?
        delete old_buf;
}
