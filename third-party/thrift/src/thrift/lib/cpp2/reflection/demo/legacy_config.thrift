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

typedef map<string, string> legacy_config

const legacy_config example = {
  "host-name": "localhost",
  "host-port": "80",
  "client-name": "my_client",
  "socket-send-timeout": "100",
  "socket-receive-timeout": "120",
  "transport-frame-size": "1024",
  "apply-compression": "1",
  "log-sampling-rate": ".01",
};
