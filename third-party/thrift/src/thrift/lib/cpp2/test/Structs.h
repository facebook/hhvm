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

#pragma once
#include <cstdlib>

template <typename Struct>
Struct create();

template <typename F>
void populateMap(F&& getter) {
  std::srand(1);
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      for (int k = 0; k < 10; k++) {
        for (int l = 0; l < 10; l++) {
          for (int m = 0; m < 10; m++) {
            getter(i, j, k, l, m, std::rand());
          }
        }
      }
    }
  }
}

#include <thrift/lib/cpp2/test/ProtoBufStructs-inl.h>
#include <thrift/lib/cpp2/test/ThriftStructs-inl.h>
