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

struct host_address {
  1: string name (from_flat = "host_name");
  2: i16 port (from_flat = "host_port");
}

struct network_timeout {
  1: i32 send (from_flat = "send_timeout");
  2: i32 receive (from_flat = "receive_timeout");
}

struct transport_config {
  1: i32 frame_size (from_flat = "frame_size");
  2: bool compress (from_flat = "compress");
}

struct nested_config {
  1: host_address address;
  2: string client_name (from_flat = "client_name");
  3: network_timeout timeout;
  4: transport_config transport;
  5: double log_rate (from_flat = "log_rate");
}

const nested_config example = {
  "address": {"name": "localhost", "port": 80},
  "client_name": "my_client",
  "timeout": {"send": 100, "receive": 120},
  "transport": {"frame_size": 1024, "compress": 1},
  "log_rate": .01,
};
