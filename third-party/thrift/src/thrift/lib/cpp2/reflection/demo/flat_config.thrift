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

namespace cpp2 static_reflection.demo

include "thrift/annotation/thrift.thrift"

struct flat_config {
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"property": "host-name"}}
  1: string host_name;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"property": "host-port"}}
  2: i16 host_port;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"property": "client-name"}}
  3: string client_name;
  @thrift.DeprecatedUnvalidatedAnnotations{
    items = {"property": "socket-send-timeout"},
  }
  4: i32 send_timeout;
  @thrift.DeprecatedUnvalidatedAnnotations{
    items = {"property": "socket-receive-timeout"},
  }
  5: i32 receive_timeout;
  @thrift.DeprecatedUnvalidatedAnnotations{
    items = {"property": "transport-frame-size"},
  }
  6: i32 frame_size;
  @thrift.DeprecatedUnvalidatedAnnotations{
    items = {"property": "apply-compression"},
  }
  7: bool compress;
  @thrift.DeprecatedUnvalidatedAnnotations{
    items = {"property": "log-sampling-rate"},
  }
  8: double log_rate;
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
