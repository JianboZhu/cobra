#include "cobra/net/buffer.h"

#include "cobra/net/socket_wrapper.h"

#include <errno.h>
#include <sys/uio.h>

namespace cobra {
namespace net {

const char Buffer::kCRLF[] = "\r\n";
const char Buffer::kEOL;

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int* savedErrno) {
  // saved an ioctl()/FIONREAD call to tell how much to read
  char extrabuf[65536];
  iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;

  // when there is enough space in this buffer, don't read into extrabuf.
  // by doing this, we read 128k-1 bytes at most
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = cobra::net::internal::readv(fd, vec, iovcnt);
  if (n < 0) {
    *savedErrno = errno;
  } else if (implicit_cast<size_t>(n) <= writable) {
    writerIndex_ += implicit_cast<size_t>(n);
  } else if (implicit_cast<size_t>(n) <= writable + sizeof extrabuf) {
    writerIndex_ = buffer_.size();

    // Append the data in 'extrabuf' into the 'buffer_'.
    append(extrabuf, implicit_cast<size_t>(n) - writable);
  } else {
    // n > writable + sizeof extrabuf
    // TODO(zhu): handle this case
  }

  return n;
}

}  // namespace net
}  // namespace cobra
