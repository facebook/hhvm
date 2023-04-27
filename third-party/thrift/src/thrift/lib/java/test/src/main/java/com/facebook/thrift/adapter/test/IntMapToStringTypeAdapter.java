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

import com.facebook.thrift.adapter.common.MapTypeAdapter;
import java.util.Collections;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class IntMapToStringTypeAdapter implements MapTypeAdapter<Integer, Integer, String> {
  @Override
  public String fromThrift(Map<Integer, Integer> map) {
    return map.keySet().stream()
        .map(key -> key + "=" + map.get(key))
        .collect(Collectors.joining(","));
  }

  @Override
  public Map<Integer, Integer> toThrift(String s) {
    if (s == null) {
      return null;
    }
    if ("".equals(s)) {
      return Collections.emptyMap();
    }
    return Stream.of(s.split(","))
        .map(p -> p.split("="))
        .collect(Collectors.toMap(p -> Integer.parseInt(p[0]), p -> Integer.parseInt(p[1])));
  }
}
