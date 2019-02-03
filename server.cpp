#include "server.h"


#define ASIO_STANDALONE
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_CO_AWAIT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS
#define ASIO_HAS_VARIADIC_TEMPLATES
#define ASIO_HAS_STD_FUNCTION
#define ASIO_HAS_STD_CHRONO
#define BOOST_ALL_NO_LIB
#define _WIN32_WINNT 0x0601
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include <asio/experimental.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/signal_set.hpp>
#include <asio/steady_timer.hpp>
#include <asio/write.hpp>

#pragma comment (lib, "lib/asio.lib")

using asio::ip::tcp;
using asio::experimental::awaitable;
using asio::experimental::co_spawn;
using asio::experimental::detached;
using asio::experimental::redirect_error;
namespace co_routine = asio::experimental::this_coro;

bool task_running = false;
using word = unsigned short;
word server_port = 8080;

server::server(int port)
{
  task_running = false;
  server_port = port & 0xFFFF;
}


server::~server()
{
  task_running = false;
}


awaitable<void> listener(tcp::acceptor acceptor)
{
  auto token = co_await co_routine::token();
  task_running = true;
  for (;task_running;)
  {
    co_await acceptor.async_accept(token);
  }
}


void server::listen()
{
  asio::io_context io_context(1);
  co_spawn(io_context, [&] { return listener(tcp::acceptor(io_context, { tcp::v4(), server_port })); }, detached);
  asio::signal_set signals(io_context, SIGINT, SIGTERM);
  signals.async_wait([&](auto, auto) {io_context.stop(); });
  io_context.run();
}


