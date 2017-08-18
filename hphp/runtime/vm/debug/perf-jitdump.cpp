/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+

 * perf-jitdump.c: perf agent to dump generated code into jit-<pid>.dump
 *
 * Adapted from the Oprofile code in opagent.c
 * Copyright 2007 OProfile authors
 * Jens Wilke
 * Daniel Hansel
 * Copyright IBM Corporation 2007
 */

#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/debug/oprof-jitdump.h"
#include "hphp/util/logger.h"
#include "hphp/util/cycles.h"

#include <folly/portability/SysSyscall.h>
#include <folly/portability/Time.h>
#include <folly/portability/Unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

using namespace HPHP::jit;

const char padding_bytes[7] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0'};

namespace HPHP {
namespace Debug {

static int getEMachine(JitHeader *hdr)  {
  char id[16];
  int fd;
  struct {
    uint16_t e_type;
    uint16_t e_machine;
  } info;

  fd = open("/proc/self/exe", O_RDONLY);
  if (fd == -1)  return -1;
  read(fd, id, sizeof(id));

  /* check ELF signature */
  if (id[0] != 0x7f || id[1] != 'E' || id[2] != 'L' || id[3] != 'F') {
    close(fd);
    return -1;
  }

  read(fd, &info, sizeof(info));
  close(fd);
  return 0;
}

static bool use_arch_timestamp;

/* On Win32 CLOCK_MONOTONIC is available */
/* On most newer Linux kernels CLOCK_MONOTONIC is defined in <bits/time.h> */
/* <time.h> includes <bits/time.h> */
#if defined(CLOCK_MONOTONIC)
static clockid_t perf_clk_id = CLOCK_MONOTONIC;
#else
static clockid_t perf_clk_id = CLOCK_REALTIME;
#endif
#define NSEC_PER_SEC    1000000000

static inline uint64_t timespec_to_ns(const struct timespec *ts)  {
  return ((uint64_t) ts->tv_sec * NSEC_PER_SEC) + ts->tv_nsec;
}

static inline uint64_t perfGetTimestamp(void) {
  struct timespec ts;

  /* cpuCycles returns arch TS, rdtsc value */
  if (use_arch_timestamp)  return cpuCycles();

  if (clock_gettime(perf_clk_id, &ts))  return 0;

  return timespec_to_ns(&ts);
}

/* only check if the directory exists */
static bool checkJitdumpDir(const char* dirName) {
  if (!dirName || !strlen(dirName))  return false;

  namespace bfs = boost::filesystem;
  boost::system::error_code ec;
  bfs::path dirPath(dirName);
  return bfs::is_directory(dirPath, ec);
}

void DebugInfo::initPerfJitDump() {
  /* create and open the jitdump file*/
  auto const jitdump_dir = RuntimeOption::EvalPerfJitDumpDir.data();

  if (checkJitdumpDir(jitdump_dir)) {
    m_perfJitDumpName = folly::sformat("{}/jit-{}.dump", jitdump_dir, getpid());
  } else {
    m_perfJitDumpName = folly::sformat("/tmp/jit-{}.dump", getpid());
  }
  Logger::Info("jitdump file: %s", m_perfJitDumpName.c_str());

  /* env variable JITDUMP_USE_ARCH_TIMESTAMP is used in the perf tools
   * framework as well, in order to collect Intel PT processor trace
   * this must be set to 1.
   * If use perf in sampling mode, there's 2 clock sources available
   * Monotonic clock : perf record -k mono ..
   *                 : perf record -k 1 ..
   * ARCH clock: perf record ..
   * The policy used here, defaults to ARCH CPU clock unless
   * the env variable is explicitly set to 0.
   */
  use_arch_timestamp = true;

  const char* str = getenv("JITDUMP_USE_ARCH_TIMESTAMP");
  if (str && *str == '0') {
    use_arch_timestamp = false;
  }

  /* check if perf_clk_id is supported else exit
   * jitdump records need to be timestamp'd
   */
  if (!perfGetTimestamp()) {
    if (use_arch_timestamp) {
      Logger::Error("system arch timestamp not supported");
    } else {
      Logger::Error("kernel does not support (monotonic) %d clk_id",
              perf_clk_id);
    }
    Logger::Error("Cannot create %s", m_perfJitDumpName.c_str());
    return;
  }

  int fd = open(m_perfJitDumpName.c_str(), O_CREAT|O_TRUNC|O_RDWR, 0666);
  if (fd < 0) {
    Logger::Error("Failed to create the file %s for perf jit dump: %s",
                     m_perfJitDumpName.c_str(), strerror(errno));
    return;
  }

  m_perfMmapMarker = mmap(nullptr,
                          sysconf(_SC_PAGESIZE),
                          PROT_READ|PROT_EXEC,
                          MAP_PRIVATE,
                          fd, 0);

  if (m_perfMmapMarker == MAP_FAILED) {
    Logger::Error("Failed to create mmap marker file for perf");
    close(fd);
    return;
  }

  m_perfJitDump = fdopen(fd, "w+");

  /* Init the jitdump header and write to the file */
  JitHeader header{};

  if (getEMachine(&header)) {
    Logger::Error("failed to get machine ELF information");
    fclose(m_perfJitDump);
  }

  header.magic      = JITHEADER_MAGIC;
  header.version    = JITHEADER_VERSION;
  header.total_size = sizeof(header);
  header.pid        = getpid();

  int padding_count;
  /* calculate amount of padding '\0' */
  padding_count = PADDING_8ALIGNED(header.total_size);
  header.total_size += padding_count;

  header.timestamp = perfGetTimestamp();

  if (use_arch_timestamp) {
    header.flags |= JITDUMP_FLAGS_ARCH_TIMESTAMP;
  }

  fwrite(&header, sizeof(header), 1, m_perfJitDump);

  /* write padding '\0' if necessary */
  if (padding_count) {
    fwrite(padding_bytes, padding_count, 1, m_perfJitDump);
  }

  fflush(m_perfJitDump);
}

void DebugInfo::closePerfJitDump() {
  if (!m_perfJitDump)  return;

  JitRecCodeClose rec;

  rec.p.id = JitRecordType::JIT_CODE_CLOSE;
  rec.p.total_size = sizeof(rec);
  rec.p.timestamp = perfGetTimestamp();

  fwrite(&rec, sizeof(rec), 1, m_perfJitDump);
  fflush(m_perfJitDump);
  fclose(m_perfJitDump);

  m_perfJitDump = nullptr;
  if (m_perfMmapMarker != MAP_FAILED) {
    munmap(m_perfMmapMarker, sysconf(_SC_PAGESIZE));
  }
}

/*
 * This is invoked from MCGenerator::recordGdbTranslation()
 * it holds the write lease, hence there is no need to acquire
 * the lock when writing to jit-<pid>.dump
 */
int DebugInfo::perfJitDumpTrace(const void* startAddr,
                                const unsigned int size,
                                const char* symName)  {

  if (!startAddr || !size) return -1;

  static int code_generation = 1;
  JitRecCodeLoad rec;
  size_t padding_count;

  uint64_t vma = reinterpret_cast<uintptr_t>(startAddr);

  rec.p.id           = JitRecordType::JIT_CODE_LOAD;
  rec.p.total_size   = sizeof(rec) + strlen(symName) + 1;
  padding_count      = PADDING_8ALIGNED(rec.p.total_size);
  rec.p.total_size  += padding_count;
  rec.p.timestamp    = perfGetTimestamp();

  rec.code_size     = size;
  rec.vma           = vma;
  rec.code_addr     = vma;
  rec.pid           = getpid();
  rec.tid           = (pid_t)syscall(FOLLY_SYS_gettid);
  rec.p.total_size += size;

  rec.code_index = code_generation++;

  /* write the name of the function and the jitdump record  */
  fwrite(&rec, sizeof(rec), 1, m_perfJitDump);
  fwrite(symName, (strlen(symName)+1), 1, m_perfJitDump);

  /* write padding '\0' if necessary */
  if (padding_count) {
    fwrite(padding_bytes, padding_count, 1, m_perfJitDump);
  }

  /* write the code generated for the tracelet */
  fwrite(startAddr, size, 1, m_perfJitDump);

  fflush(m_perfJitDump);
  return 0;
}

} // namespace Debug
} // namespace HPHP
