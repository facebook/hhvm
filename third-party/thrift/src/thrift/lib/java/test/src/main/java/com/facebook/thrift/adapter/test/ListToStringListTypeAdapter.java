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

package com.facebook.thrift.adapter.test;

import com.facebook.thrift.adapter.common.ListTypeAdapter;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class ListToStringListTypeAdapter implements ListTypeAdapter<List<Integer>, String> {
  @Override
  public String fromThrift(List<List<Integer>> list) {
    return list.stream()
        .map(i -> i.stream().map(String::valueOf).collect(Collectors.joining(",")))
        .collect(Collectors.joining(":"));
  }

  @Override
  public List<List<Integer>> toThrift(String s) {
    return s == null
        ? null
        : Stream.of(s.split(":"))
            .map(i -> Stream.of(i.split(",")).map(Integer::parseInt).collect(Collectors.toList()))
            .collect(Collectors.toList());
  }
}
