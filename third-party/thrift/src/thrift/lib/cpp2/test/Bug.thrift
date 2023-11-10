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

namespace cpp2 apache.thrift.test

exception Start {} // default one

safe exception FirstBlood {}

stateful client exception DoubleKill {}

safe permanent client exception TripleKill {}

service Bug {
  void fun1() throws (1: Start s);

  void fun2() throws (1: FirstBlood f);

  void fun3() throws (1: DoubleKill d);

  void fun4() throws (1: TripleKill t);

  void fun5() throws (1: TripleKill t);

  void fun6() throws (1: TripleKill t);
}
