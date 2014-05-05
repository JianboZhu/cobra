#ifndef COBRA_BASE_EXCEPTION_H_
#define COBRA_BASE_EXCEPTION_H_

#include <base/Types.h>
#include <exception>

namespace cobra
{

class Exception : public std::exception
{
 public:
  explicit Exception(const char* what);
  explicit Exception(const string& what);
  virtual ~Exception() throw();
  virtual const char* what() const throw();
  const char* stackTrace() const throw();

 private:
  void fillStackTrace();

  string message_;
  string stack_;
};

}

#endif  // COBRA_BASE_EXCEPTION_H_
