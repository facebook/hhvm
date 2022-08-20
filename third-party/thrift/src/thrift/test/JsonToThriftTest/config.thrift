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

namespace cpp2 apache.thrift.test
namespace java thrift.test

struct Watchdog {
  1: string type;
  2: string name;
  3: string entity;
  4: string key;
  5: double threshold;
  6: bool reverse;
}

struct WhitelistEntry {
  1: string name;
  2: string type;
  3: list<string> contacts;
  4: string target_kernel;
  5: string target;
  6: string rebooter;
  7: string rate;
  8: i32 atonce;
  9: list<Watchdog> watchdogs;
}

struct Config {
  1: map<string, string> vars;
  2: list<WhitelistEntry> whitelist;
}
