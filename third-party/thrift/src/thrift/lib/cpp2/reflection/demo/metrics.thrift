/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 static_reflection.demo

struct host_info {
  1: i32 cpu;
  2: i32 memory;
  3: i32 disk;
  4: i32 network_bytes_sent;
  5: i32 network_bytes_received;
  6: i32 network_retransmits;
  7: i32 network_connection_reset;
  8: i32 network_packets_dropped;
}
