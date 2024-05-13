/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/core.h>
#include <folly/String.h>

#include "watchman/Connect.h"
#include "watchman/Logging.h"
#include "watchman/PDU.h"
#include "watchman/PerfSample.h"
#include "watchman/Shutdown.h"
#include "watchman/telemetry/LogEvent.h"
#include "watchman/telemetry/WatchmanStructuredLogger.h"
#include "watchman/watchman_stream.h"

namespace watchman {
namespace {

// Work-around decodeNext which implictly resets to non-blocking
std::optional<json_ref>
decodeNext(watchman_stream* client, PduBuffer& buf, json_error_t& jerr) {
  client->setNonBlock(false);
  return buf.decodeNext(client, &jerr);
}

/* Periodically connect to our endpoint and verify that we're talking
 * to ourselves.  This is normally a sign of madness, but if we don't
 * get an answer, or get a reply from someone else, we know things
 * are bad; someone removed our socket file or there was some kind of
 * race condition that resulted in multiple instances starting up.
 */
void check_my_sock(watchman_stream* client) {
  auto cmd = json_array({typed_string_to_json("get-pid", W_STRING_UNICODE)});
  PduBuffer buf;
  json_error_t jerr;
  pid_t my_pid = ::getpid();

  auto res = buf.pduEncodeToStream(PduFormat{is_bser, 0}, cmd, client);
  if (res.hasError()) {
    log(watchman::FATAL,
        "Failed to send get-pid PDU: ",
        folly::errnoStr(res.error()),
        "\n");
    /* NOTREACHED */
  }

  buf.clear();
  auto result = decodeNext(client, buf, jerr);
  if (!result) {
    log(watchman::FATAL,
        "Failed to decode get-pid response: ",
        jerr.text,
        " ",
        folly::errnoStr(errno),
        "\n");
    /* NOTREACHED */
  }

  auto pid = result->get_optional("pid");
  if (!pid) {
    log(watchman::FATAL,
        "Failed to get pid from get-pid response: ",
        jerr.text,
        "\n");
    /* NOTREACHED */
  }
  auto remote_pid = pid->asInt();

  if (remote_pid != my_pid) {
    log(watchman::FATAL,
        "remote pid from get-pid ",
        long(remote_pid),
        " doesn't match my pid (",
        (long)my_pid,
        "\n");
    /* NOTREACHED */
  }
}

/**
 * Run clock command for the specified root. Useful for getting time
 * information.
 */
void check_clock_command(watchman_stream* client, const json_ref& root) {
  PduBuffer buf;
  json_error_t jerr;

  auto cmd = json_array(
      {typed_string_to_json("clock", W_STRING_UNICODE),
       root,
       json_object({{"sync_timeout", json_integer(20000)}})});
  auto res = buf.pduEncodeToStream(PduFormat{is_bser, 0}, cmd, client);
  if (res.hasError()) {
    throw std::runtime_error(fmt::format(
        "Failed to send clock PDU: {}", folly::errnoStr(res.error())));
  }

  buf.clear();
  auto result = decodeNext(client, buf, jerr);
  if (!result) {
    throw std::runtime_error(fmt::format(
        "Failed to decode clock response: {} {}",
        jerr.text,
        folly::errnoStr(errno)));
  }

  // Check for error in the response
  auto error = result->get_optional("error");
  if (error) {
    throw std::runtime_error(
        fmt::format("Clock error : {}", json_to_w_string(*error)));
  }

  // We use presence of "clock" as success
  auto clock = result->get_optional("clock");
  if (!clock) {
    throw std::runtime_error("Failed to get clock in response");
  }
}

/**
 * Runs watch-list command and returns a json_ref that contains a list of roots.
 */
json_ref get_watch_list(watchman_stream* client) {
  auto cmd = json_array({typed_string_to_json("watch-list", W_STRING_UNICODE)});
  PduBuffer buf;
  json_error_t jerr;

  auto res = buf.pduEncodeToStream(PduFormat{is_bser, 0}, cmd, client);
  if (res.hasError()) {
    throw std::runtime_error(fmt::format(
        "Failed to send watch-list PDU: {}", folly::errnoStr(res.error())));
  }

  buf.clear();
  auto result = decodeNext(client, buf, jerr);
  if (!result) {
    throw std::runtime_error(fmt::format(
        "Failed to decode watch-list response: {} error:  {}",
        jerr.text,
        folly::errnoStr(errno)));
  }
  return result->get("roots");
}

/**
 * Run watch-list to get the list of watched roots. Then, run 'clock' on each
 * watched root. We perf log the time taken to get the clock.
 */
void do_clock_check(watchman_stream* client) {
  // We don't expect errors in these calls. However, we do want to make sure
  // they are not fatal.
  try {
    auto roots = get_watch_list(client);
    for (auto& r : roots.array()) {
      ClockTest clockTest;
      clockTest.root = r.toString();
      PerfSample sample("clock-test");
      sample.add_meta("root", json_object({{"path", r}}));
      try {
        check_clock_command(client, r);
      } catch (const std::exception& ex) {
        log(watchman::ERR, "Failed do_clock_check : ", ex.what(), "\n");
        clockTest.error = ex.what();
        sample.add_meta("error", w_string_to_json(ex.what()));
        sample.force_log();
      }

      if (sample.finish()) {
        sample.log();
      }

      const auto& [samplingRate, eventCount] =
          getLogEventCounters(LogEventType::ClockTestType);
      // Log if override set, or if we have hit the sample rate
      if (sample.will_log || eventCount == samplingRate) {
        clockTest.event_count = eventCount != samplingRate ? 0 : eventCount;
        getLogger()->logEvent(clockTest);
      }
    }
  } catch (const std::exception& ex) {
    // Catch std::domain_error and std::runtime_error
    log(watchman::ERR, "Failed get_watch_list : ", ex.what(), "\n");
  }
}

void sanityCheckThread() noexcept {
  w_set_thread_name("sanitychecks");
  auto lastCheck = std::chrono::steady_clock::now();
  auto interval = std::chrono::minutes(1);

  log(ERR, "starting sanityCheckThread\n");
  // we want to try the checks in here once per minute, but since
  // this is mildly ghetto we don't have a way to directly signal
  // this thread when we're shutting down.  So we sleep for a second
  // at a time and then check to see if a shutdown is in progress
  // so that we can shutdown with a slightly lower latency than
  // if we were to sleep for a minute at a time.
  while (!w_is_stopping()) {
    auto now = std::chrono::steady_clock::now();
    if (now - lastCheck < interval) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    lastCheck = now;
    log(DBG, "running sanity checks\n");

    auto client = w_stm_connect(6000);
    if (client.hasError()) {
      log(watchman::FATAL,
          "Failed to connect to myself for sanity check: ",
          folly::errnoStr(client.error()),
          "\n");
      /* NOTREACHED */
    }
    check_my_sock(client.value().get());
    do_clock_check(client.value().get());
  }
  log(ERR, "done with sanityCheckThread\n");
}
} // namespace

void startSanityCheckThread() {
  // The blocking pipe reads we use on win32 can cause us to get blocked
  // forever running the sanity checks, so skip this on win32
#ifndef _WIN32
  std::thread thr(sanityCheckThread);
  thr.detach();
#endif
}
} // namespace watchman
