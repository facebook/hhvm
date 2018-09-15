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
*/

#include "hphp/tools/hfsort/jitsort.h"

#ifndef _MSC_VER
#include "hphp/util/light-process.h"
#endif
#include "hphp/util/logger.h"
#include "hphp/util/string-vsnprintf.h"

#include <folly/Format.h>
#include <folly/ScopeGuard.h>

#include "hphp/tools/hfsort/hfutil.h"

#include <assert.h>
#include <zlib.h>
#include <ctype.h>

namespace HPHP { namespace hfsort {

constexpr uint32_t kPageSize = 2 << 20;

typedef std::map<uint32_t,std::vector<TargetId>> Group2BaseMap;

void error(const char* msg) {
  Logger::Error("JitSort: %s\n", msg);
  throw std::exception();
}

static bool readLine(std::string& out, FILE* file) {
  out.resize(0);
  while (true) {
    int c = fgetc(file);
    if (c == EOF) return out.size();
    if (c == '\n') return true;
    out.push_back(c);
  }
}

static void readPerfMap(CallGraph& cg, Group2BaseMap& f2b, FILE* file) {
  std::string line;
  uint64_t addr;
  uint64_t end;
  int n;
  while (readLine(line, file)) {
    if (sscanf(line.c_str(), "%" SCNx64 "%" SCNx64 "%n",
               &addr, &end, &n) >= 2) {
      auto pos = line.rfind(' ');
      if (pos == n++) {
        // arc or fixup metadata
        continue;
      }
      auto id = folly::to<int>(line.substr(n, pos - n));
      if (cg.addFunc(line.substr(n), addr, end - addr, id)) {
        auto funcId = cg.funcs.size();
        auto& group = f2b[id];
        group.push_back(funcId);
        if (group.size() > 1) {
          cg.targets[group[0]].size += cg.targets[funcId].size;
        }
      }
    }
  }
}

static TargetId getTargetId(const CallGraph& cg, const char* perfLine) {
  uint64_t addr;
  int ret = sscanf(perfLine, "%" SCNx64, &addr);
  if (ret != 1) return InvalidId;
  return cg.addrToTargetId(addr);
}

static void readPerfHits(CallGraph& cg, Group2BaseMap& f2b, FILE* file) {
  std::string line;

  while (readLine(line, file)) {
    HFTRACE(2, "readPerfHits: line: %s\n", line.c_str());
    if (!line.size()) continue;
    if (line[0] == '#') continue;
    if (isspace(line[0])) continue;

    if (!readLine(line, file)) error("reading perf data");
    TargetId idTop = getTargetId(cg, line.c_str());
    if (idTop == InvalidId) continue;
    idTop = f2b[cg.funcs[idTop].group][0];
    cg.targets[idTop].samples++;
    HFTRACE(2, "readPerfHits: idTop: %u %s\n", idTop,
            cg.funcs[idTop].mangledNames[0].c_str());
    if (!readLine(line, file)) error("reading perf data");
    TargetId idCaller = getTargetId(cg, line.c_str());
    if (idCaller != InvalidId) {
      idCaller = f2b[cg.funcs[idCaller].group][0];
      cg.incArcWeight(idCaller, idTop);
      HFTRACE(2, "readPerfData: idCaller: %u %s\n", idCaller,
              cg.funcs[idCaller].mangledNames[0].c_str());
    }
  }

  // Normalize incoming arc weights for each node.
  for (TargetId f = 0; f < cg.targets.size(); f++) {
    auto& func = cg.targets[f];
    double inWeight = 0;
    for (auto src : func.preds) {
      auto& arc = *cg.arcs.find(Arc(src, f));
      auto& pred = cg.targets[src];
      inWeight += arc.weight * pred.samples / pred.succs.size();
    }
    if (!inWeight) inWeight = 1;
    for (auto src : func.preds) {
      auto& arc = *cg.arcs.find(Arc(src, f));
      auto& pred = cg.targets[src];
      arc.normalizedWeight =
        arc.weight * pred.samples / (inWeight * pred.succs.size());
    }
  }
}

static void print(CallGraph& cg, Group2BaseMap& f2b,
                  const std::vector<Cluster>& clusters, FILE* outfile) {
  uint32_t totalSize = 0;
  uint32_t curPage   = 0;
  uint32_t hotfuncs  = 0;
  HFTRACE(1, "============== page 0 ==============\n");
  for (auto& cluster : clusters) {
    if (!cluster.samples) continue;
    HFTRACE(1, "------------ density = %.3lf (%u / %u) -----------\n",
            (double) cluster.samples / cluster.size,
            cluster.samples, cluster.size);
    for (auto const gid : cluster.targets) {
      const auto& group = f2b[cg.funcs[gid].group];
      assert(group[0] == gid);
      // We added the sizes of all members of the group to
      // the group leader. Fix things up again.
      for (auto const fid : group) {
        if (fid != gid) cg.targets[gid].size -= cg.targets[fid].size;
      }
      for (auto const fid : group) {
        const auto& func = cg.targets[fid];
        hotfuncs++;
        const auto& mangledName = cg.funcs[fid].mangledNames.back();
        fprintf(outfile, "%" PRIx64 " %" PRIx64 " %s\n",
                cg.funcs[fid].addr, cg.funcs[fid].addr + func.size,
                mangledName.c_str());

        HFTRACE(1, "start = %6" PRIu32 " : %s\n", totalSize,
                cg.toString(fid).c_str());
        totalSize += func.size;
        uint32_t newPage = totalSize / kPageSize;
        if (newPage != curPage) {
          curPage = newPage;
          HFTRACE(1,
                  "============== page %" PRIu32 " ==============\n",
                  curPage);
        }
      }
    }
  }
  Logger::Info("Number of hot translations: %u\n  Number of clusters: %lu\n",
               hotfuncs, clusters.size());
}

#ifndef _MSC_VER
static bool light_exec(std::string cmd, std::string& out) {
  auto* proc = LightProcess::popen(cmd.c_str(), "r", "/");
  if (!proc) return false;
  int ret = 1;
  {
    SCOPE_EXIT {
      ret = LightProcess::pclose(proc);
      if (WIFEXITED(ret)) ret = WEXITSTATUS(ret);
    };
    std::string line;
    while (readLine(line, proc)) {
      out += line + '\n';
    }
  }
  return !ret;
}
#endif

int jitsort(int pid, int time, FILE* perfSymFile, FILE* relocResultsFile) {
  bool skipPerf = pid < 0;
  if (pid < 0) pid = -pid;

  CallGraph cg;

  Group2BaseMap f2b;
  readPerfMap(cg, f2b, perfSymFile);

  std::vector<Cluster> clusters;
#ifndef _MSC_VER
  if (time < 0) {
#endif
    TargetId id = 0;
    for (auto& f : cg.targets) {
      f.samples = 1;
      clusters.emplace_back(id++, f);
    }
#ifndef _MSC_VER
  } else {
    auto perfData = folly::sformat("/tmp/perf-{}.data", pid);
    auto perfHitsFileName = folly::sformat("/tmp/perf-{}.out", pid);

    auto perfCmd =
        "perf record -BN --no-buffering -g -p {0} -e instructions -o {1} -- sleep {2} && "
        "perf script -i {1} --fields comm,ip > {3}";

    auto cmds = std::string("sh -c '(") + perfCmd + ") 2>&1'";
    cmds = folly::sformat(cmds, pid, perfData, time, perfHitsFileName);

    std::string err;
    if (!skipPerf && !light_exec(cmds, err)) {
      error(folly::sformat("Failed to run `{}': {}\n", cmds, err).c_str());
    }
    if (FILE* perfHitsFile = fopen(perfHitsFileName.c_str(), "r")) {
      SCOPE_EXIT { fclose(perfHitsFile); };
      readPerfHits(cg, f2b, perfHitsFile);
    } else {
      error("Error opening perf data file\n");
    }

    clusters = clusterize(cg);
    sort(clusters.begin(), clusters.end(), compareClustersDensity);
  }
#endif
  print(cg, f2b, clusters, relocResultsFile);

  return 0;
}

} }
