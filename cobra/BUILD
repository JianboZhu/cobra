# Author: Jianbo Zhu (jianbo.zhujb@gmail.com)
#
# The blade BUILD file.

cc_library(
  name = 'acceptor',
  srcs = 'acceptor.cpp',
  deps = [
    ':channel',
    ':endpoint',
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
    ':endpoint',
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'endpoint',
  srcs = 'endpoint.cpp',
  deps = [
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'server',
  srcs = 'server.cpp',
  deps = [
    ':acceptor',
    ':worker',
    ':worker_thread_pool',
    ':socket_wrapper',
    ':tcp_connection',
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
    ':worker',
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
    ':worker',
    ':endpoint',
    ':socket_wrapper',
  ]
)

cc_library(
  name = 'tcp_server',
  srcs = 'tcp_server.cpp',
  deps = [
    ':acceptor',
    ':worker',
    ':worker_thread_pool',
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

cc_library(
  name = 'worker',
  srcs = 'worker.cpp',
  deps = [
    ':channel',
    ':socket_wrapper',
    ':timer_queue',
  ]
)

cc_library(
  name = 'worker_thread',
  srcs = 'worker_thread.cpp',
  deps = [
    ':worker',
  ]
)

cc_library(
  name = 'worker_thread_pool',
  srcs = 'worker_thread_pool.cpp',
  deps = [
    ':worker',
    ':worker_thread',
  ]
)
