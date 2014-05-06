#ifndef COBRA_INSPECT_PROCESSINSPECTOR_H_
#define COBRA_INSPECT_PROCESSINSPECTOR_H_

#include <cobra/inspect/Inspector.h>
#include <boost/noncopyable.hpp>

namespace cobra {

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

}  // namespace cobra

#endif  // COBRA_INSPECT_PROCESSINSPECTOR_H_
