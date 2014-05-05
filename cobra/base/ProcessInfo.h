#ifndef COBRA_BASE_PROCESSINFO_H_
#define COBRA_BASE_PROCESSINFO_H_

#include <cobra/base/Types.h>
#include <cobra/base/Timestamp.h>
#include <vector>

namespace cobra
{

namespace ProcessInfo
{
  pid_t pid();
  string pidString();
  uid_t uid();
  string username();
  uid_t euid();
  Timestamp startTime();

  string hostname();
  string procname();

  /// read /proc/self/status
  string procStatus();

  /// read /proc/self/stat
  string procStat();

  /// readlink /proc/self/exe
  string exePath();

  int openedFiles();
  int maxOpenFiles();

  int numThreads();
  std::vector<pid_t> threads();
}

}

#endif  // COBRA_BASE_PROCESSINFO_H_
