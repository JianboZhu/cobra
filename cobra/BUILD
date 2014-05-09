# Author: Jianbo Zhu (jianbo.zhujb@gmail.com)
#
# The blade BUILD file.

cc_library(
  name = 'acceptor',
  srcs = 'acceptor.cpp',
  deps = [
    ':channel',
    ':inet_address',
    ':socket',
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'buffer',
  srcs = 'buffer.cpp',
  deps = [
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'channel',
  srcs = 'channel.cpp',
  deps = [
  ]
)

cc_library(
  name = 'connector',
  srcs = 'connector.cpp',
  deps = [
    ':channel',
    ':inet_address',
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'event_loop',
  srcs = 'event_loop.cpp',
  deps = [
    ':channel',
    ':socket_wrapper',
    ':timer_queue',
  ]
)

cc_library(
  name = 'event_loop_thread',
  srcs = 'event_loop_thread.cpp',
  deps = [
    ':event_loop',
  ]
)

cc_library(
  name = 'event_loop_thread_pool',
  srcs = 'event_loop_thread_pool.cpp',
  deps = [
    ':event_loop',
    ':event_loop_thread',
  ]
)

cc_library(
  name = 'inet_address',
  srcs = 'inet_address.cpp',
  deps = [
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'server',
  srcs = 'server.cpp',
  deps = [
    ':acceptor',
    ':event_loop',
    ':event_loop_thread_pool',
    ':socket_wrapper',
    ':tcp_connection',
  ]
)

cc_library(
  name = 'socket',
  srcs = 'socket.cpp',
  deps = [
    ':inet_address',
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'socket_wrapper',
  srcs = 'socket_wrapper.cpp',
  deps = [
  ]
)

cc_library(
  name = 'tcp_client',
  srcs = 'tcp_client.cpp',
  deps = [
    ':connector',
    ':event_loop',
    ':socket_wrapper',
    ':tcp_connection',
  ]
)

cc_library(
  name = 'tcp_connection',
  srcs = 'tcp_connection.cpp',
  deps = [
    ':buffer',
    ':channel',
    ':event_loop',
    ':inet_address',
    ':socket',
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'tcp_server',
  srcs = 'tcp_server.cpp',
  deps = [
    ':acceptor',
    ':event_loop',
    ':event_loop_thread_pool',
    ':socket_wrapper',
    ':tcp_connection',
  ]
)

cc_library(
  name = 'timer',
  srcs = 'timer.cpp',
  deps = [
  ]
)

cc_library(
  name = 'timer_queue',
  srcs = 'timer_queue.cpp',
  deps = [
    ':channel',
    ':timer',
  ]
)
