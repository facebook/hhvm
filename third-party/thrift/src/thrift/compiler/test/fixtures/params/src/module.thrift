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

namespace java.swift test.fixtures.params

service NestedContainers {
  void mapList(1: map<i32, list<i32>> foo);
  void mapSet(1: map<i32, set<i32>> foo);
  void listMap(1: list<map<i32, i32>> foo);
  void listSet(1: list<set<i32>> foo);

  void turtles(1: list<list<map<i32, map<i32, set<i32>>>>> foo);
}
