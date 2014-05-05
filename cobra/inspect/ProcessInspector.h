#ifndef COBRA_NET_INSPECT_PROCESSINSPECTOR_H_
#define COBRA_NET_INSPECT_PROCESSINSPECTOR_H_

#include <cobra/net/inspect/Inspector.h>
#include <boost/noncopyable.hpp>

namespace cobra
{
namespace net
{

class ProcessInspector : boost::noncopyable
{
 public:
  void registerCommands(Inspector* ins);

  static string overview(HttpRequest::Method, const Inspector::ArgList&);
  static string pid(HttpRequest::Method, const Inspector::ArgList&);
  static string procStatus(HttpRequest::Method, const Inspector::ArgList&);
  static string openedFiles(HttpRequest::Method, const Inspector::ArgList&);
  static string threads(HttpRequest::Method, const Inspector::ArgList&);

  static string username_;
};

}
}

#endif  // COBRA_NET_INSPECT_PROCESSINSPECTOR_H_
