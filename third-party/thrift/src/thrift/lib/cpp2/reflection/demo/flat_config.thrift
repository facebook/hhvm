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

struct flat_config {
  1: string host_name (property = "host-name");
  2: i16 host_port (property = "host-port");
  3: string client_name (property = "client-name");
  4: i32 send_timeout (property = "socket-send-timeout");
  5: i32 receive_timeout (property = "socket-receive-timeout");
  6: i32 frame_size (property = "transport-frame-size");
  7: bool compress (property = "apply-compression");
  8: double log_rate (property = "log-sampling-rate");
}

const flat_config example = {
  "host_name": "localhost",
  "host_port": 80,
  "client_name": "my_client",
  "send_timeout": 100,
  "receive_timeout": 120,
  "frame_size": 1024,
  "compress": 1,
  "log_rate": .01,
};
