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
 * Wire-format-compatible mirror of fb303_status from
 * fb303/thrift/fb303_core.thrift. Values must remain in lockstep — container
 * scheduler health checks and debug RPC clients expect the same i32 codes
 * regardless of which server stack answered.
 */
enum fast_fb303_status {
  DEAD = 0,
  STARTING = 1,
  ALIVE = 2,
  STOPPING = 3,
  STOPPED = 4,
  WARNING = 5,
}

/**
 * fast_thrift counterpart of `service Status` in
 * `common/thrift/thrift/status.thrift`. Method set is intentionally kept
 * in lockstep with that file — when adding a method to one, add it to the
 * other. Lives in a separate IDL because `@cpp.FastServer` is exclusive
 * with the legacy SvIf codegen.
 *
 * Container scheduler health checks call `getStatus()` on this interface;
 * debug RPC clients render it in their server-health view.
 *
 * NOTE: All methods defined here are exempt from ACL enforcement by the
 * surrounding service framework. Thus, these methods must be read-only and
 * expose no sensitive information.
 */
@cpp.FastServer
service Status {
  /**
   * Gets the status of this service.
   */
  fast_fb303_status getStatus();

  /**
   * User friendly description of status, such as why the service is in
   * the dead or warning state, or what is being started or stopped.
   */
  string getStatusDetails();

  /**
   * Returns the 1-minute load average on the system (i.e. the load may not
   * all be coming from the current process).
   */
  double getLoad();
}
