/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <folly/Synchronized.h>
#include <folly/dynamic.h>
#include <folly/json.h>
#include <watchman/cppclient/WatchmanClient.h>

#include "hphp/runtime/base/configs/autoload.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/watchman.h"
#include "hphp/util/struct-log.h"

namespace HPHP {
namespace {

void logSlowQuery(const watchman::QueryResult& res,
                  const folly::dynamic& queryObj,
                  ClockTime preLockTime,
                  ClockTime preExecTime,
                  ClockTime execTime) {
  auto const finTime = std::chrono::steady_clock::now();
  auto const dur = [] (auto s, auto e) {
    return
      std::chrono::duration_cast<std::chrono::milliseconds>(e - s)
      .count();
  };

  auto const incDur = dur(preLockTime, finTime);
  if (incDur < RO::EvalLogSlowWatchmanQueriesMsec ||
      !StructuredLog::coinflip(RO::EvalLogSlowWatchmanQueriesRate)) {
    return;
  }

  auto const runDur = dur(execTime, finTime);
  auto const schedDur = dur(preExecTime, execTime);
  auto const lockDur = dur(preLockTime, preExecTime);

  StructuredLogEntry ent;
  ent.setInt("sample_rate", RO::EvalLogSlowWatchmanQueriesRate);
  ent.setInt("duration_ms", incDur);
  ent.setInt("run_ms", runDur);
  ent.setInt("schedule_ms", schedDur);
  ent.setInt("lock_ms", lockDur);
  ent.setStr("query", folly::toJson(queryObj));

  // These aren't really watchman specific but it's primarily autoload
  // related queries that we're interested in.
  ent.setInt("autoload_db_can_create", Cfg::Autoload::DBCanCreate);
  ent.setStr("autoload_db_path", Cfg::Autoload::DBPath);

  auto const since = queryObj.get_ptr("since");
  auto const suffix = queryObj.get_ptr("suffix");
  auto const glob = queryObj.get_ptr("glob");
  auto const path = queryObj.get_ptr("path");
  auto const all = queryObj.get_ptr("all");
  auto const fields = queryObj.get_ptr("fields");
  auto const expression = queryObj.get_ptr("expression");
  auto const lockTimeout = queryObj.get_ptr("lock_timeout");
  auto const syncTimeout = queryObj.get_ptr("sync_timeout");
  auto const caseSensitive = queryObj.get_ptr("case_sensitive");
  auto const emptyOnFresh = queryObj.get_ptr("empty_on_fresh_instance");

  ent.setInt("empty_on_fresh",
             emptyOnFresh && emptyOnFresh->isBool() &&
             emptyOnFresh->asBool());

  ent.setInt("case_sensitive",
             caseSensitive && caseSensitive->isBool() &&
             caseSensitive->asBool());

  if (lockTimeout && lockTimeout->isInt()) {
    ent.setInt("lock_timeout_ms", lockTimeout->asInt());
  }

  if (syncTimeout && syncTimeout->isInt()) {
    ent.setInt("sync_timeout_ms", syncTimeout->asInt());
  }

  if (expression) {
    ent.setStr("expression", folly::toJson(*expression));
  }

  std::vector<folly::StringPiece> gens;
  if (since) gens.emplace_back("since");
  if (suffix) gens.emplace_back("suffix");
  if (glob) gens.emplace_back("glob");
  if (path) gens.emplace_back("path");
  if (all) gens.emplace_back("all");
  ent.setVec("generators", gens);

  auto const logList = [&] (auto ptr, auto listName) {
    std::vector<folly::StringPiece> lst;
    std::vector<std::string> strs;
    if (ptr->isString()) {
      strs.emplace_back(ptr->asString());
      lst.emplace_back(strs.back());
    } else if (ptr->isArray()) {
      strs.reserve(ptr->size());
      lst.reserve(ptr->size());
      for (auto const& s : *ptr) {
        if (s.isString()) {
          strs.emplace_back(s.asString());
          lst.emplace_back(strs.back());
        }
      }
    }
    ent.setVec(listName, lst);
  };
  if (suffix) logList(suffix, "suffix_generators");
  if (glob)   logList(glob, "glob_generators");
  if (path)   logList(path, "path_generators");
  if (fields) logList(fields, "fields");

  if (since && since->isString()) {
    ent.setStr("clock_generator", since->asString());
  } else if (since && since->isObject()) {
    auto const scm = since->get_ptr("scm");
    auto const mergebase = scm && scm->isObject()
      ? scm->get_ptr("mergebase") : nullptr;
    auto const nestedClock = since->get_ptr("clock");

    if (nestedClock && nestedClock->isString()) {
      ent.setStr("clock_generator", nestedClock->asString());
    }
    if (mergebase && mergebase->isString()) {
      ent.setStr("mergebase_generator", mergebase->asString());
    }
    if (scm && scm->isObject()) {
      ent.setStr("scm_generator", folly::toJson(*scm));
    }
  }

  std::string clockStr;
  auto const files = res.raw_.get_ptr("files");
  auto const clock = res.raw_.get_ptr("clock");
  auto const fresh = res.raw_.get_ptr("is_fresh_instance");

  ent.setInt("is_fresh", fresh && fresh->isBool() && fresh->asBool());
  if (files && files->isArray()) {
    ent.setInt("num_files", files->size());
  }
  if (clock) {
    if (clock->isString()) clockStr = clock->asString();
    else clockStr = folly::toJson(*clock);
    ent.setStr("result_clock", clockStr);
  }

  StructuredLog::log("hhvm_slow_watchman", ent);
}

InitFiniNode registerLogger(
  [] {
    if (RO::EvalLogSlowWatchmanQueriesRate) {
      Watchman::setProfiler(logSlowQuery);
    }
  },
  InitFiniNode::When::ProcessInit
);

}}
