#ifndef COBRA_NET_BUFFER_H_
#define COBRA_NET_BUFFER_H_

#include "base/StringPiece.h"
#include "base/Types.h"

#include <assert.h>
#include <string.h>
//#include <unistd.h>  // ssize_t

#include <algorithm>
#include <vector>

#include "cobra/endian.h"

namespace cobra {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  Buffer()
    : buffer_(kCheapPrepend + kInitialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend) {
    assert(readableBytes() == 0);
    assert(writableBytes() == kInitialSize);
    assert(prependableBytes() == kCheapPrepend);
  }

  void swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }

  inline size_t readableBytes() const {
    return writerIndex_ - readerIndex_;
  }

  size_t writableBytes() const {
    return buffer_.size() - writerIndex_;
  }

  inline size_t prependableBytes() const {
    return readerIndex_;
  }

  // Point to the location of the first readable character.
  inline const char* BeginRead() const {
    return begin() + readerIndex_;
  }

  // Find the "\r\n" symbol.
  const char* findCRLF() const {
    const char* crlf = std::search(BeginRead(), BeginWrite(), kCRLF, kCRLF+2);
    return crlf == BeginWrite() ? NULL : crlf;
  }

  const char* findCRLF(const char* start) const {
    assert(BeginRead() <= start);
    assert(start <= BeginWrite());
    const char* crlf = std::search(start, BeginWrite(), kCRLF, kCRLF+2);
    return crlf == BeginWrite() ? NULL : crlf;
  }

  const char* findEOL() const {
    const void* eol = memchr(BeginRead(), '\n', readableBytes());
    return static_cast<const char*>(eol);
  }

  const char* findEOL(const char* start) const {
    assert(BeginRead() <= start);
    assert(start <= BeginWrite());
    const void* eol = memchr(start, '\n', readableBytes());
    return static_cast<const char*>(eol);
  }

  // retrieve returns void, to prevent
  // string str(retrieve(readableBytes()), readableBytes());
  // the evaluation of two functions are unspecified
  void retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len < readableBytes()) {
      readerIndex_ += len;
    } else {
      retrieveAll();
    }
  }

  void retrieveUntil(const char* end) {
    assert(BeginRead() <= end);
    assert(end <= BeginWrite());
    retrieve(end - BeginRead());
  }

  void retrieveInt32() {
    retrieve(sizeof(int32_t));
  }

  void retrieveInt16() {
    retrieve(sizeof(int16_t));
  }

  void retrieveInt8() {
    retrieve(sizeof(int8_t));
  }

  void retrieveAll() {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
  }

  string retrieveAllAsString() {
    return retrieveAsString(readableBytes());;
  }

  string retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    string result(BeginRead(), len);
    // We didn't clear the retrieved contnent in the buffer by
    // just move the index.
    retrieve(len);
    return result;
  }

  StringPiece toStringPiece() const {
    return StringPiece(BeginRead(), static_cast<int>(readableBytes()));
  }

  void append(const StringPiece& str) {
    append(str.data(), str.size());
  }

  void append(const char* /*restrict*/ data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data+len, BeginWrite());
    hasWritten(len);
  }

  void append(const void* /*restrict*/ data, size_t len) {
    append(static_cast<const char*>(data), len);
  }

  void ensureWritableBytes(size_t len) {
    if (writableBytes() < len)
    {
      makeSpace(len);
    }
    assert(writableBytes() >= len);
  }

  // Point to the location of the first writeable character.
  char* BeginWrite() {
    return begin() + writerIndex_;
  }

  const char* BeginWrite() const {
    return begin() + writerIndex_;
  }

  void hasWritten(size_t len) {
    writerIndex_ += len;
  }

  ///
  /// Append int32_t using network endian
  ///
  void appendInt32(int32_t x) {
    int32_t be32 = hostToNetwork32(x);
    append(&be32, sizeof be32);
  }

  void appendInt16(int16_t x) {
    int16_t be16 = hostToNetwork16(x);
    append(&be16, sizeof be16);
  }

  void appendInt8(int8_t x) {
    append(&x, sizeof x);
  }

  ///
  /// Read int32_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int32_t readInt32() {
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
  }

  int16_t readInt16() {
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
  }

  int8_t readInt8() {
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
  }

  ///
  /// Peek int32_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int32_t peekInt32() const {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, BeginRead(), sizeof be32);
    return networkToHost32(be32);
  }

  int16_t peekInt16() const {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, BeginRead(), sizeof be16);
    return networkToHost16(be16);
  }

  int8_t peekInt8() const {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *BeginRead();
    return x;
  }

  ///
  /// Prepend int32_t using network endian
  ///
  void prependInt32(int32_t x) {
    int32_t be32 = hostToNetwork32(x);
    prepend(&be32, sizeof be32);
  }

  void prependInt16(int16_t x) {
    int16_t be16 = hostToNetwork16(x);
    prepend(&be16, sizeof be16);
  }

  void prependInt8(int8_t x) {
    prepend(&x, sizeof x);
  }

  void prepend(const void* /*restrict*/ data, size_t len) {
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, begin() + readerIndex_);
  }

  void shrink(size_t reserve) {
    // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
    Buffer other;
    other.ensureWritableBytes(readableBytes() + reserve);
    other.append(toStringPiece());
    swap(other);
  }

  size_t internalCapacity() const {
    return buffer_.capacity();
  }

  /// Read data directly into buffer.
  ///
  /// It may implement with readv(2)
  /// @return result of read(2), @c errno is saved
  ssize_t readFd(int fd, int* savedErrno);

 private:
  char* begin() {
    return &*buffer_.begin();
  }

  const char* begin() const {
    return &*buffer_.begin();
  }

  void makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
      // FIXME: move readable data
      buffer_.resize(writerIndex_ + len);
    } else {
      // move readable data to the front, make space inside buffer
      assert(kCheapPrepend < readerIndex_);
      size_t readable = readableBytes();
      std::copy(begin() + readerIndex_,
                begin() + writerIndex_,
                begin() + kCheapPrepend);
      readerIndex_ = kCheapPrepend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == readableBytes());
    }
  }

 private:
  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;

  // "\r\n": Carriage-Return(\r, means "return") Line-Feed(\n, means "new line")
  static const char kCRLF[];

  // End of a line.
  static const char kEOL = '\n';
};

}  // namespace cobra

#endif  // COBRA_NET_BUFFER_H_
