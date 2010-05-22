/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <runtime/base/rtti_info.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/externals.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/runtime_error.h>
#include <util/lock.h>
#include <util/util.h>
#include <compiler/option.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

RTTIInfo RTTIInfo::TheRTTIInfo;

RTTIInfo::RTTIInfo() : m_loaded(false), m_count(0), m_profData(NULL) {
}

void RTTIInfo::translate_rtti(const char *rttiDirectory) {
  if (!loadProfData(rttiDirectory)) return;
  for (int i = 0; i < m_count; i++) {
    int total = 0;
    for (int j = 0; j < MaxNumDataTypes; j++) total += m_profData[i][j];
    if (!total) continue;
    printf("%s(%d):", m_id2name[i].c_str(), total);
    if (m_profData[i][getDataTypeIndex(KindOfNull)]) {
      printf(" n/%u", m_profData[i][getDataTypeIndex(KindOfNull)]);
    }
    if (m_profData[i][getDataTypeIndex(KindOfBoolean)]) {
      printf(" b/%u", m_profData[i][getDataTypeIndex(KindOfBoolean)]);
    }
    int totalInt = m_profData[i][getDataTypeIndex(KindOfByte)] +
                   m_profData[i][getDataTypeIndex(KindOfInt16)] +
                   m_profData[i][getDataTypeIndex(KindOfInt32)] +
                   m_profData[i][getDataTypeIndex(KindOfInt64)];
    if (totalInt) printf(" i/%u", totalInt);
    if (m_profData[i][getDataTypeIndex(KindOfDouble)]) {
      printf(" d/%u", m_profData[i][getDataTypeIndex(KindOfDouble)]);
    }
    int totalStr = m_profData[i][getDataTypeIndex(LiteralString)] +
                   m_profData[i][getDataTypeIndex(KindOfString)];
    if (totalStr) printf(" s/%u", totalStr);
    if (m_profData[i][getDataTypeIndex(KindOfArray)]) {
      printf(" a/%u", m_profData[i][getDataTypeIndex(KindOfArray)]);
    }
    if (m_profData[i][getDataTypeIndex(KindOfObject)]) {
      printf(" o/%u", m_profData[i][getDataTypeIndex(KindOfObject)]);
    }
    if (m_profData[i][getDataTypeIndex(KindOfVariant)]) {
      printf(" v/%u", m_profData[i][getDataTypeIndex(KindOfVariant)]);
    }
    printf("\n");
  }
  free(m_profData);
  m_profData = NULL;
}

class RTTICounters : public RequestEventHandler {
public:
  RTTICounters() : m_data(NULL), m_count(0), m_requests(0) { }
  RTTICounter *getCounter(int id) { return m_data ? &m_data[id] : NULL; }
  virtual void requestInit() {
    if (!m_data) {
      m_count = RTTIInfo::TheRTTIInfo.getCount();
      if (m_count > 0) {
        m_data = (RTTICounter *)calloc(m_count, sizeof(RTTICounter));
      }
    }
  }
  virtual void requestShutdown() {
    m_requests++;
    if (strcmp(RuntimeOption::ExecutionMode, "svr") == 0 &&
        m_requests % 10) return;
    // write rtti profile data every 10 requests
    if (m_data) {
      char path[PATH_MAX];
      snprintf(path, sizeof(path), "%s%d/%llx.rtti",
               RuntimeOption::RTTIDirectory.c_str(),
               getpid(), (unsigned long long)pthread_self());
      int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (fd == -1) {
        raise_warning("%s", Util::safe_strerror(errno).c_str());
      } else {
        if (write(fd, m_data, (m_count * sizeof(RTTICounter))) < 0) {
          // an error occured but we're in shutdown already, so ignore
        }
        close(fd);
      }
    }
  }
private:
  RTTICounter *m_data;
  int m_count;
  unsigned int m_requests;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(RTTICounters, s_rtti_counters);

unsigned int *getRTTICounter(int id) {
  return (unsigned int *)(s_rtti_counters->getCounter(id));
}

void RTTIInfo::init(bool createDir) {
  Lock lock(m_mutex);
  if (!m_loaded) {
    loadParamMap(g_paramrtti_map);
    if (m_count > 0) {
      char path[PATH_MAX];
      snprintf(path, sizeof(path), "%s%d/",
               RuntimeOption::RTTIDirectory.c_str(), getpid());
      if (createDir) Util::mkdir(path);
    }
    m_loaded = true;
  }
  if (m_count == 0) m_count = m_id2name.size();
}

void RTTIInfo::loadParamMap(const char **p) {
  m_count = 0;
  while (*p) {
    const char *source = *p++;
    m_count++;
    m_id2name.push_back(source);
  }
}

void RTTIInfo::loadMetaData(const char *filename) {
  ASSERT(!m_loaded);
  FILE *f = fopen(filename, "r");
  if (f == NULL) return;
  char line[1024];
  if (fgets(line, sizeof(line), f)) {
    sscanf(line, "%d", &m_count);
  }
  while (fgets(line, sizeof(line), f)) {
    int len = strlen(line);
    ASSERT(len > 0);
    if (line[len-1] == '\n') line[len-1] = 0;
    m_functions.insert(line);
  }
  fclose(f);
}

bool RTTIInfo::loadProfData(const char *rttiDirectory) {
  std::vector <std::string> rttiFiles;
  dirent* de;
  DIR* dp;

  init(false);
  int size = m_count * sizeof(RTTICounter);
  ASSERT(rttiDirectory);
  dp = opendir(rttiDirectory);
  if (!dp) return false;

  while (true) {
    de = readdir( dp );
    if (de == NULL) break;
    struct stat st;
    string path(rttiDirectory);
    path += "/";
    path += de->d_name;
    if (stat(path.c_str(), &st) == 0 &&
        (st.st_mode & S_IFMT) == S_IFREG &&
        st.st_size == size) {
      rttiFiles.push_back(path);
    }
  }
  closedir( dp );
  if (rttiFiles.size() == 0) return false;

  RTTICounter *sum = (RTTICounter *)calloc(m_count, sizeof(RTTICounter));
  RTTICounter *counter = (RTTICounter *)malloc(size);
  for (unsigned int i = 0; i < rttiFiles.size(); i++) {
    int fd = open(rttiFiles[i].c_str(), O_RDONLY);
    if (fd == -1) {
      raise_warning("%s", Util::safe_strerror(errno).c_str());
      continue;
    }
    if (read(fd, counter,  size) != size) {
      raise_warning("%s", Util::safe_strerror(errno).c_str());
      close(fd);
      continue;
    }
    RTTICounter *cs, *cc;
    for (cs = sum, cc = counter; cs < sum + m_count; cs++, cc++) {
      for (int j = 0; j < MaxNumDataTypes; j++) {
        (*cs)[j] += (*cc)[j];
      }
    }
    close(fd);
  }
  free(counter);
  if (m_profData) free(m_profData);
  m_profData = sum;
  return true;
}

bool RTTIInfo::exists(const char *funcName) {
  return m_functions.find(funcName) != m_functions.end();
}

///////////////////////////////////////////////////////////////////////////////
}
