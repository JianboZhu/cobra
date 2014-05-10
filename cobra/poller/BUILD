# Author: Jianbo Zhu

cc_library(
  name = 'default_poller',
  srcs = 'default_poller.cpp',
  deps = [
    ':epoll_poller',
    ':poll_poller',
  ]
)

cc_library(
  name = 'epoll_poller',
  srcs = 'epoll_poller.cpp',
  deps = [
    '//cobra:channel',
  ]
)

cc_library(
  name = 'poll_poller',
  srcs = 'poll_poller.cpp',
  deps = [
    '//cobra:channel',
  ]
)
