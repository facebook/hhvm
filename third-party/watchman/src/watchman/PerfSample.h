/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/SysTime.h>
#include <string>
#include <vector>

#include "watchman/thirdparty/jansson/jansson.h"

// Performance metrics sampling

namespace watchman {

/**
 * Contains metadata regarding root used in structured logging.
 */
struct RootMetadata {
  w_string root_path;
  int64_t recrawl_count;
  bool case_sensitive;
  w_string watcher;
};

template <typename T>
void addRootMetadataToEvent(const RootMetadata& root_metadata, T& event) {
  event.root = root_metadata.root_path.string();
  event.recrawl = root_metadata.recrawl_count;
  event.case_sensitive = root_metadata.case_sensitive;
  event.watcher = root_metadata.watcher.string();
}

class PerfSample {
 public:
  // What we're sampling across
  const char* description;

  // Additional arbitrary information.
  // This is a json object with various properties set inside it
  json_ref meta_data{json_object()};

  // Measure the wall time
  timeval time_begin;
  timeval time_end;
  timeval duration;

  // If set to true, the sample should be sent to the logging
  // mechanism
  bool will_log{false};

  // If non-zero, force logging on if the wall time is greater
  // that this value
  double wall_time_elapsed_thresh{0};

  double get_perf_sampling_thresh() const;

#ifdef HAVE_SYS_RESOURCE_H
  // When available (posix), record these process-wide stats.
  // It can be difficult to attribute these directly to the
  // action being sampled because there can be multiple
  // watched roots and these metrics include the usage from
  // all of them.
  struct rusage usage_begin;
  struct rusage usage_end;
  struct rusage usage;
#endif

  /**
   * Initialize and mark the start of a sample.
   * The given description is an unowned pointer - it must live as long as the
   * PerfSample.
   */
  explicit PerfSample(const char* description);

  PerfSample(const PerfSample&) = delete;
  PerfSample(PerfSample&&) = delete;
  PerfSample& operator=(const PerfSample&) = delete;
  PerfSample& operator=(PerfSample&&) = delete;

  // Augment any configuration policy and cause this sample to be logged if the
  // walltime exceeds the specified number of seconds (fractions are supported)
  void set_wall_time_thresh(double thresh);

  // Mark the end of a sample.  Returns true if the policy is to log this
  // sample.  This allows the caller to conditionally build and add metadata
  bool finish();

  // Annotate sample with Root metadata.
  void add_root_metadata(const RootMetadata& root_metadata);

  // Annotate the sample with metadata
  void add_meta(const char* key, json_ref&& val);

  // Force the sample to go to the log
  void force_log();

  // If will_log is set, arranges to send the sample to the log
  void log();
};

void perf_shutdown();

void processSamples(
    size_t argv_limit,
    size_t maximum_batch_size,
    std::vector<json_ref>& samples,
    std::function<void(std::vector<std::string>)> command_line,
    std::function<void(std::string)> single_large_sample);

} // namespace watchman

/* vim:ts=2:sw=2:et:
 */
