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

#include "hphp/runtime/base/tracing.h"

#include "hphp/runtime/base/init-fini-node.h"

#include "hphp/runtime/server/cli-server.h"

#include "hphp/util/assertions.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/service-data.h"

#include "folly/Random.h"

#ifdef HHVM_FACEBOOK
#include "common/base/BuildInfo.h"
#include "common/fbwhoami/FbWhoAmI.h"
#endif

#include <memory>
#include <mutex>

namespace HPHP {
namespace tracing {

namespace {

// Global table mapping (canonicalized) URLs to a count for that URL
hphp_fast_string_map<size_t> s_requests_per_url;
// Global request count
size_t s_total_requests = 0;
// Protects both of the above
std::mutex s_requests_lock;

// Some URLs will include request data, making them effectively always
// unique. We want to treat all those URLs as the same, so do some
// very basic canonicalization. Remove anything from the URL after the
// ".php" (if present). We can add more cases as necessary.
std::string canonicalizeURL(const std::string& url) {
  auto const pos = url.rfind(".php");
  if (pos == std::string::npos) return url;
  return url.substr(0, pos+4);
}

// Determine if we should gather aggregate statistics for this
// request. Return a tuple containing (1) whether we should trace, (2)
// the total request count corresponding to this request, (3) the
// request count for this particular URL, (4) the sample rate used for
// this request. The later three fields will be recorded into the
// StructuredLogEntry for this request.
std::tuple<bool, size_t, size_t, size_t> shouldRun(folly::StringPiece url) {
  // We can avoid taking the lock if we know we'll never succeed.
  if (!RuntimeOption::EvalTracingSampleRate &&
      !RuntimeOption::EvalTracingFirstRequestsCount &&
      !RuntimeOption::EvalTracingPerRequestCount) {
    return std::make_tuple(false, 0, 0, 0);
  }

  // Otherwise look at the global table to get the request counts:
  auto const counts = [&] () -> std::pair<size_t, size_t> {
    std::lock_guard<std::mutex> _{s_requests_lock};
    return std::make_pair(
      s_total_requests++,
      s_requests_per_url[canonicalizeURL(url.toString())]++
    );
  }();

  // Take the min of the rates which apply for this request and do a
  // coinflip:
  auto rate = std::numeric_limits<uint32_t>::max();
  if (RuntimeOption::EvalTracingSampleRate) {
    rate = std::min(rate, RuntimeOption::EvalTracingSampleRate);
  }
  if (counts.first < RuntimeOption::EvalTracingFirstRequestsCount) {
    rate = std::min(rate, RuntimeOption::EvalTracingFirstRequestsSampleRate);
  }
  if (counts.second < RuntimeOption::EvalTracingPerRequestCount) {
    rate = std::min(rate, RuntimeOption::EvalTracingPerRequestSampleRate);
  }
  if (!rate || rate == std::numeric_limits<uint32_t>::max()) {
    return std::make_tuple(false, counts.first, counts.second, 0);
  }

  return std::make_tuple(
    StructuredLog::coinflip(rate),
    counts.first,
    counts.second,
    rate
  );
}

// Set some common fields we want in every StructuredLogEntry. The
// entries don't change, so we cache them to avoid overhead.
void setCommonFields(StructuredLogEntry& entry) {
  static auto const values = [&] {
    hphp_fast_string_map<std::string> v;
#ifdef HHVM_FACEBOOK
    v["build_compiler"] = BuildInfo_kCompiler;
    v["build_mode"] = BuildInfo_kBuildMode;
    v["server_type"] = facebook::FbWhoAmI::getServerTypeArch();
    v["region"] = facebook::FbWhoAmI::getRegion();
#endif
    v["debug"] = debug ? "true" : "false";
    v["lowptr"] = use_lowptr ? "true" : "false";
    v["repo_auth"] = RuntimeOption::RepoAuthoritative ? "true" : "false";
    v["is_server"] = RuntimeOption::ServerExecutionMode() ? "true" : "false";
    v["is_cli_server"] = is_cli_server_mode() ? "true" : "false";
    v["use_jit"] = RuntimeOption::EvalJit ? "true" : "false";
    v["tag_id"] = RuntimeOption::EvalTracingTagId;
    return v;
  }();

  for (auto const& kv : values) {
    entry.setStr(kv.first, kv.second);
  }
}

}

namespace detail {

THREAD_LOCAL_FLAT(RequestState, tl_active);
std::unique_ptr<RequestImplFactory> s_factory;

// Start a request, returning a new RequestState, or nullptr if we
// determined we will not trace this request.
RequestState* startRequestImpl(folly::StringPiece name,
                               folly::StringPiece url,
                               folly::StringPiece group) {
  // Request should not be active:
  assertx(tl_active.isNull());
  auto const counts = shouldRun(url);
  // Check if we want to trace this request
  if (!std::get<0>(counts)) return nullptr;
  // Set up the request state and create the top block
  auto active = tl_active.getCheck();
  assertx(!active->m_impl);
  assertx(active->m_blocks.empty());
  // If we have a registered backend, check if the backend wants to
  // trace this request.
  active->m_impl = s_factory ? s_factory->start(name) : nullptr;
  active->m_url = url.toString();
  active->m_requestCount = std::get<1>(counts);
  active->m_perURLRequestCount = std::get<2>(counts);
  active->m_sampleRate = std::get<3>(counts);
  active->m_blocks.emplace_back();
  active->m_requestGroup = group;
  auto& block = active->m_blocks.back();
  block.m_name = name.toString();
  block.m_start = Clock::now();
  return active;
}

// Start a new block, taking care of only the non-backend portion (the
// backend portion is handled in the header so we can optionally call
// the lambda for the properties).
BlockState& startBlockNoTraceImpl(folly::StringPiece name) {
  assertx(!tl_active.isNull());
  auto& active = *tl_active;
  assertx(!active.m_blocks.empty());
  active.m_blocks.emplace_back();
  auto& block = active.m_blocks.back();
  block.m_name = name.toString();
  block.m_start = Clock::now();
  return block;
}

// Stop the current block and gather aggregate statistics for it.
void stopBlockImpl() {
  assertx(!tl_active.isNull());
  auto& active = *tl_active;
  assertx(!active.m_blocks.empty());
  auto& current = active.m_blocks.back();

  // Calculate the inclusive and exclusive time spent with the
  // block. The inclusive time is just the delta between the start and
  // end times. The exclusive time is the inclusive time minus time
  // spent in it's children.
  auto const inclusive = Clock::now() - current.m_start;
  auto const exclusive = inclusive - current.m_childDuration;

  auto& stats = active.m_stats[current.m_name];

  // Update the exclusive stats:
  ++stats.m_exclusive_count;
  stats.m_exclusive_total += exclusive;
  stats.m_exclusive_max = std::max(stats.m_exclusive_max, exclusive);

  // Blocks with the same name can be nested within each other (if we
  // have recursion for example). This isn't a problem for exclusive
  // time since that only counts time spent within the block
  // itself. However it leads to double counting with inclusive
  // time. If this block has a block with the same name as a parent,
  // ignore it for the purposes of inclusive time calculation.
  auto const doubleCount = [&] {
    for (size_t i = active.m_blocks.size() - 1; i > 0; --i) {
      if (active.m_blocks[i-1].m_name == current.m_name) return true;
    }
    return false;
  }();
  if (!doubleCount) {
    ++stats.m_inclusive_count;
    stats.m_inclusive_total += inclusive;
    stats.m_inclusive_max = std::max(stats.m_inclusive_max, inclusive);
  }

  // Count any points added to this block (but no timestamps).
  for (auto const& kv : current.m_points) {
    stats.m_points[kv.first] += kv.second;
  }

  // Popping the block implicitly destroys the BlockImpl (if any),
  // which signals the tracing backend to deal with the block ending.
  active.m_blocks.pop_back();

  // If this block has a parent, record it's inclusive time in the
  // parent so the parent can calculate it's exclusive time properly.
  if (!active.m_blocks.empty()) {
    active.m_blocks.back().m_childDuration += inclusive;
  } else {
    // Otherwise its the top block, so it's inclusive time should be
    // the time of the entire request.
    active.m_total += inclusive;
  }
}

BlockState& addPointNoTraceImpl(folly::StringPiece name) {
  assertx(!tl_active.isNull());
  auto& active = *tl_active;
  assertx(!active.m_blocks.empty());
  auto& block = active.m_blocks.back();
  ++block.m_points[name.toString()];
  return block;
}

}

// Stop the current request, aggregating all the data and sending it
// out via StructuredLog.
void stopRequest() {
  if (detail::tl_active.isNull()) return;
  auto& active = *detail::tl_active;

  // Stop the top block, providing aggregation fo the entire request.
  detail::stopBlockImpl();

  auto const toUS = [] (detail::Clock::duration d) {
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
  };

  // We've kept statistics for the blocks aggregated by name. We'll
  // now emit a StructuredLogEntry for each one. Generate an arbitrary
  // request-id to link all of these entries together into the same
  // request.

  auto const requestID = folly::Random::rand64(0, 1ULL << 63);

  auto const canonicalized = canonicalizeURL(active.m_url);
  for (auto const& kv : active.m_stats) {
    StructuredLogEntry entry;
    entry.setStr("url", active.m_url);
    entry.setStr("canonical_url", canonicalized);
    entry.setInt("request_id", requestID);
    entry.setInt("request_count", active.m_requestCount);
    entry.setInt("sample_rate", active.m_sampleRate);
    entry.setInt("request_count_per_url", active.m_perURLRequestCount);
    entry.setInt("total_request_time_us", toUS(active.m_total));
    // Also log the tracing backend request id if present
    if (active.m_impl) entry.setStr("trace_id", active.m_impl->getID());

    entry.setStr("block", kv.first);
    entry.setInt("total_inclusive_us", toUS(kv.second.m_inclusive_total));
    entry.setInt("total_exclusive_us", toUS(kv.second.m_exclusive_total));
    entry.setInt("max_inclusive_us", toUS(kv.second.m_inclusive_max));
    entry.setInt("max_exclusive_us", toUS(kv.second.m_exclusive_max));
    entry.setInt("inclusive_count", kv.second.m_inclusive_count);
    entry.setInt("exclusive_count", kv.second.m_exclusive_count);
    entry.setInt(
      "avg_inclusive_us",
      toUS(kv.second.m_inclusive_total) / kv.second.m_inclusive_count);
    entry.setInt(
      "avg_exclusive_us",
      toUS(kv.second.m_exclusive_total) / kv.second.m_exclusive_count);
    entry.setInt(
      "basis_points_inclusive_us",
      10000 * toUS(kv.second.m_inclusive_total) / toUS(active.m_total));
    entry.setInt(
      "basis_points_exclusive_us",
      10000 * toUS(kv.second.m_exclusive_total) / toUS(active.m_total));
    entry.setProcessUuid("hhvm_uuid");

    for (auto const& point : kv.second.m_points) {
      entry.setInt(point.first, point.second);
    }

    if (!active.m_requestGroup.empty()) {
      entry.setStr("request_group", active.m_requestGroup);
    }

    // Add the fields which don't change and send the entry out.
    setCommonFields(entry);
    StructuredLog::log("hhvm_tracing_stats", entry);
  }

  // This destroys the RequestImpl and therefore lets the tracing
  // backend know the request is over.
  detail::tl_active.destroy();
}

void setFactory(std::unique_ptr<RequestImplFactory> f) {
  assertx(!detail::s_factory);
  detail::s_factory = std::move(f);
}

static InitFiniNode initTracingTagIdCounter([] {
  if (RuntimeOption::EvalTracingTagId.empty()) return;
  auto counter = ServiceData::createCounter(
    folly::sformat("vm.tracing_tag_id.{}", RuntimeOption::EvalTracingTagId)
  );
  counter->setValue(1);
}, InitFiniNode::When::PostRuntimeOptions);

}}
