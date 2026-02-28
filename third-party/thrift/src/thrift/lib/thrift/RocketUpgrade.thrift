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

namespace cpp2 apache.thrift
namespace py3 thrift.lib.thrift
namespace go 'github.com/facebook/fbthrift/thrift/lib/thrift/rocket_upgrade'

/**
 * This interface should be used by the clients that want to attempt upgrading
 * an existing Thrift connection to a Rocket/RSocket connection.
 */
service RocketUpgrade {
  /**
   * This request initiates the upgrade sequence. Client MUST send this and
   * only this request immediately after establishing a new Thrift connection
   * if it wants to attempt using the upgrade protocol. No additional requests
   * MUST NOT be sent over a connection until a response to this request is
   * received.
   * If a client receives a non-error response to this request it MUST assume
   * that the upgrade was successful and the connection MUST be treated as a
   * newly established RSocket connection. If a client received any exception
   * as a response to this request it MUST continue using the connection
   * using the original Thrift protocol of that connection or close the
   * connection.
   * If the server supports the RocketUpgrade protocol it SHOULD respond to the
   * upgrade request with a valid response. Once such response is written into
   * the connection the server MUST assume that the upgrade was successful and
   * the connection MUST be treated as a newly established RSocket connection.
   * If a server has any other inflight requests over the same connection when
   * such upgrade request is received it MUST reject the upgrade request.
   */
  void upgradeToRocket();
}
