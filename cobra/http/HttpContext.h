#ifndef COBRA_H_TTP_HTTPCONTEXT_H
#define COBRA_H_TTP_HTTPCONTEXT_H

#include <base/copyable.h>

#include <cobra/http/HttpRequest.h>

namespace cobra {

class HttpContext : public cobra::copyable {
 public:
  enum HttpRequestParseState
  {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  HttpContext()
    : state_(kExpectRequestLine)
  {
  }

  // default copy-ctor, dtor and assignment are fine

  bool expectRequestLine() const
  { return state_ == kExpectRequestLine; }

  bool expectHeaders() const
  { return state_ == kExpectHeaders; }

  bool expectBody() const
  { return state_ == kExpectBody; }

  bool gotAll() const
  { return state_ == kGotAll; }

  void receiveRequestLine()
  { state_ = kExpectHeaders; }

  void receiveHeaders()
  { state_ = kGotAll; }  // FIXME

  void reset()
  {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }

  const HttpRequest& request() const
  { return request_; }

  HttpRequest& request()
  { return request_; }

 private:
  HttpRequestParseState state_;
  HttpRequest request_;
};

}  // namespace cobra

#endif  // COBRA_H_TTP_HTTPCONTEXT_H
