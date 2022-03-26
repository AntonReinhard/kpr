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

#include <l4/libgfxbitmap/font.h>

#include <l4/cxx/iostream>
#include <l4/cxx/ipc_stream>
#include <l4/cxx/string.h>
#include <l4/cxx/string>
#include <l4/re/video/goos>
#include <l4/re/util/video/goos_fb>

using namespace L4Re::Util::Video;
using namespace L4Re::Video;

static L4Re::Util::Registry_server<> server;

bool exiting = false;

// some different colors
unsigned colors[] = {0x0000FF00, 0x000000FF, 0x0000000F, 0x0000F000, 0x00000F00, 0x000000F0, 0x0000000F};
int clientCounter = 0;
constexpr unsigned bgColor = 0x00000000;

// higher scroll -> scrolled down
int scroll = 0;

void* base;
View::Info info;
// tuple of color + log message
std::vector<std::tuple<unsigned, std::string>> log;
std::mutex logMutex;

void clearScreen();
void renderLoop();
void memTest();

class LoggingServer : public L4::Epiface_t<LoggingServer, Logger> {
public:
    LoggingServer(const L4::String& id)
        : id(strdup(id.p_str()))
        , colorId(clientCounter++ % (sizeof(colors)/sizeof(colors[0]))) {
        std::string text = "Client with ID " + std::string(id.p_str(), id.length()) + " created";
        std::unique_lock<std::mutex> lock(logMutex);
        log.emplace_back(std::tuple<unsigned, std::string>{colors[colorId], text});
    }

    ~LoggingServer() {
        delete id.p_str();
    }

    int op_print(Logger::Rights, L4::Ipc::String<> s) {
        std::string string = "[" + std::string(this->id.p_str(), this->id.length()) + "]: " + std::string(s.data);
        std::unique_lock<std::mutex> lock(logMutex);
        log.emplace_back(std::tuple<unsigned, std::string>{colors[colorId], string});
        return 0;
    }

private:
    L4::String id;
    int colorId;
};

class SessionServer : public L4::Epiface_t<SessionServer, L4::Factory> {
public:
    int op_create(L4::Factory::Rights, L4::Ipc::Cap<void>& res, l4_mword_t type, L4::Ipc::Varg_list_ref args) {
        if (type != 0) {
            return -L4_ENODEV;
        }

        int i = 0;
        L4::String tag;
        for (L4::Ipc::Varg arg : args) {
            if (arg.is_of<char const*>()) {
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

void memTest() {

    std::vector<int*> vectors;
    vectors.reserve(1000);
    L4::cout << "reserved vector\n";

    for (int i = 0; i < 100; ++i) {
        auto n = new int[10]();
        vectors.emplace_back(new int[100]());
        delete[] n;
    }
    for (int i = 0; i < 100; ++i) {
        delete[] vectors.at(i);
    }
}

void renderLoop() {
    constexpr int line_height = 13;
    static int logLength = 0;

    while (!exiting) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        std::unique_lock<std::mutex> lock(logMutex);
        if (log.size() == logLength) {
            continue;
        }
        logLength = log.size();

        clearScreen();
        auto line = info.height / line_height - 1;
        
        std::string scrolltext = "Scroll: " + std::to_string(scroll);
        gfxbitmap_font_text(base, reinterpret_cast<l4re_video_view_info_t*>(&info), GFXBITMAP_DEFAULT_FONT, scrolltext.c_str(), scrolltext.length(), info.width - 80, 0, 0x0000FFFF, bgColor);

        for (auto it = log.rbegin(); it != log.rend(); ++it, --line) {
            int yCoord = line * line_height + scroll;
            if (yCoord < 0) {
                break;
            }
            if (yCoord > static_cast<int>(info.height)) {
                continue;
            }
            auto& color = std::get<0>(*it);
            auto& str = std::get<1>(*it);
            gfxbitmap_font_text(base, reinterpret_cast<l4re_video_view_info_t*>(&info), GFXBITMAP_DEFAULT_FONT, str.c_str(), str.length(), 5, yCoord, color, bgColor);
        }
    }
}

void clearScreen() {
    // Paint it black!
    for (auto y = 0; y < info.height; ++y) {
        for (auto x = 0; x < info.width; ++x) {
            auto addr = base + y * (info.pixel_info.bytes_per_pixel() * info.width) + 
                            x * info.pixel_info.bytes_per_pixel();

            *static_cast<unsigned*>(addr) = bgColor;
        }
    }
}

int main() {
    static SessionServer sserver;

    L4::cout << "memtest...\n";
    memTest();
    L4::cout << "success!\n";

    // Register session server
    if (!server.registry()->register_obj(&sserver, "logger").is_valid()) {
        L4::cout << "Could not register my service (session server), is there a 'logger' in the caps table?\n";
        return 1;
    }

    sleep(1);

    // Register Framebuffer
    Goos_fb fb("fb");
    base = fb.attach_buffer();
    
    int r = fb.view_info(&info);
    if (r != 0) {
        L4::cout << "### Couldn't get framebuffer info ###\n";
        return 0;
    }

    L4::cout << "Created Framebuffer of size (" << info.width << ", " << info.height << ") and " << info.pixel_info.bytes_per_pixel() << "B per pixel\n";

    gfxbitmap_font_init();
    std::thread t(renderLoop);

    // Wait for client requests
    server.loop();

    L4::cout << "Exiting\n";

    if (t.joinable()) {
        t.join();
    }

    return 0;
}
