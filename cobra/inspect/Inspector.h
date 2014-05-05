#ifndef COBRA_NET_INSPECT_INSPECTOR_H_
#define COBRA_NET_INSPECT_INSPECTOR_H_

#include <base/Mutex.h>
#include <cobra/http/HttpRequest.h>
#include <cobra/http/HttpServer.h>

#include <map>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace cobra {

class ProcessInspector;
class PerformanceInspector;

// An internal inspector of the running process, usually a singleton.
// Better to run in a seperated thread, as some method may block for seconds
class Inspector : boost::noncopyable
{
 public:
  typedef std::vector<string> ArgList;
  typedef boost::function<string (HttpRequest::Method, const ArgList& args)> Callback;
  Inspector(EventLoop* loop,
            const InetAddress& httpAddr,
            const string& name);
  ~Inspector();

  void add(const string& module,
           const string& command,
           const Callback& cb,
           const string& help);

 private:
  typedef std::map<string, Callback> CommandList;
  typedef std::map<string, string> HelpList;

  void start();
  void onRequest(const HttpRequest& req, HttpResponse* resp);

  HttpServer server_;
  boost::scoped_ptr<ProcessInspector> processInspector_;
  boost::scoped_ptr<PerformanceInspector> performanceInspector_;
  MutexLock mutex_;
  std::map<string, CommandList> modules_;
  std::map<string, HelpList> helps_;
};

}  // namespace cobra

#endif  // COBRA_NET_INSPECT_INSPECTOR_H_
