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

package com.facebook.thrift.swift.adapters;

public class ThriftMessageAdapters {
  public static com.facebook.thrift.protocol.TMessage adaptThriftMessage(
      org.apache.thrift.protocol.TMessage message) {
    return new com.facebook.thrift.protocol.TMessage(message.name, message.type, message.seqid);
  }

  public static org.apache.thrift.protocol.TMessage adaptThriftMessage(
      com.facebook.thrift.protocol.TMessage message) {
    return new org.apache.thrift.protocol.TMessage(message.name, message.type, message.seqid);
  }
}
