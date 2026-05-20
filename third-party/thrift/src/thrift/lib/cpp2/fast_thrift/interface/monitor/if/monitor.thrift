/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

include "thrift/annotation/cpp.thrift"

package "facebook.com/thrift/fast_thrift"

namespace cpp2 apache.thrift.fast_thrift

/**
 * fast_thrift counterpart of `service Monitor` in
 * `common/thrift/thrift/monitor.thrift`. Method set is intentionally kept
 * in lockstep with that file — when adding a method to one, add it to the
 * other. Lives in a separate IDL because `@cpp.FastServer` is exclusive
 * with the legacy SvIf codegen, so the two services cannot share a source
 * file.
 *
 * The methods exposed are a subset of those in fb303 — the subset directly
 * relevant to monitoring, which excludes dangerous methods like
 * `shutdown()`.
 *
 * NOTE: All methods defined here are ignored by ServiceFramework's
 * ACLChecker module. Thus, these methods must be read-only and expose no
 * sensitive information.
 */
@cpp.FastServer
service Monitor {
  /**
   * Gets the counters for this service
   */
  map<string, i64> getCounters();

  /**
   * Gets a subset of counters which match a
   * Perl Compatible Regular Expression for this service
   */
  map<string, i64> getRegexCounters(1: string regex);

  /**
   * Get counter values for a specific list of keys.  Returns a map from
   * key to counter value; if a requested counter doesn't exist, it won't
   * be in the returned map.
   */
  map<string, i64> getSelectedCounters(1: list<string> keys);

  /**
   * Gets the value of a single counter
   */
  i64 getCounter(1: string key);

  /**
   * Gets the exported string values for this service
   */
  map<string, string> getExportedValues();

  /**
   * Get exported strings for a specific list of keys.  Returns a map from
   * key to string value; if a requested key doesn't exist, it won't
   * be in the returned map.
   */
  map<string, string> getSelectedExportedValues(1: list<string> keys);

  /**
   * Gets a subset of exported values which match a
   * Perl Compatible Regular Expression for this service
   */
  map<string, string> getRegexExportedValues(1: string regex);

  /**
   * Gets the value of a single exported string
   */
  string getExportedValue(1: string key);

  /**
   * Returns the unix time that the server has been running since
   */
  i64 aliveSince();

  /**
   * Returns the current memory usage (RSS) of this process in bytes.
   */
  i64 getMemoryUsage();

  /**
   * Returns the pid of the process
   */
  i64 getPid();

  /**
   * Returns the command line used to execute this process.
   */
  string getCommandLine();
}
