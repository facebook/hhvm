#include <cpp/base/types.h>
#include <cpp/base/memory/smart_allocator.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(ThreadInfo, ThreadInfo::s_threadInfo);

ThreadInfo::ThreadInfo() {
  map<int, ObjectAllocatorWrapper *> &wrappers =
    ObjectAllocatorCollector::getWrappers();
  m_allocators.resize(wrappers.size());
  for (map<int, ObjectAllocatorWrapper *>::iterator it = wrappers.begin();
       it != wrappers.end(); it++) {
    m_allocators[it->first] = it->second->get();
  }

  m_top = NULL;
  m_stackdepth = 0;
  m_profiler = NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
