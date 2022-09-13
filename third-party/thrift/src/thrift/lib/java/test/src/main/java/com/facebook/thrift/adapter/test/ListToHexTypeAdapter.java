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
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class ListToHexTypeAdapter implements ListTypeAdapter<ByteBuf, String> {
  @Override
  public String fromThrift(List<ByteBuf> list) {
    return list.stream().map(ByteBufUtil::hexDump).collect(Collectors.joining(":"));
  }

  @Override
  public List<ByteBuf> toThrift(String s) {
    return s == null
        ? null
        : Stream.of(s.split(":"))
            .map(hex -> Unpooled.wrappedBuffer(ByteBufUtil.decodeHexDump(hex)))
            .collect(Collectors.toList());
  }
}
