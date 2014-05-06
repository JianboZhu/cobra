#ifndef COBRA_INSPECT_PERFORMANCEINSPECTOR_H_
#define COBRA_INSPECT_PERFORMANCEINSPECTOR_H_

#include <cobra/inspect/Inspector.h>
#include <boost/noncopyable.hpp>

namespace cobra {

class PerformanceInspector : boost::noncopyable
{
 public:
  void registerCommands(Inspector* ins);

  static string heap(HttpRequest::Method, const Inspector::ArgList&);
  static string growth(HttpRequest::Method, const Inspector::ArgList&);
  static string profile(HttpRequest::Method, const Inspector::ArgList&);
  static string cmdline(HttpRequest::Method, const Inspector::ArgList&);
  static string memstats(HttpRequest::Method, const Inspector::ArgList&);
  static string memhistogram(HttpRequest::Method, const Inspector::ArgList&);

  static string symbol(HttpRequest::Method, const Inspector::ArgList&);
};

}  // namespace cobra

#endif  // COBRA_INSPECT_PERFORMANCEINSPECTOR_H_
