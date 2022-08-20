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

package com.facebook.thrift.metadata;

public enum ThriftTransportType {
  HEADER("thrift"),
  FRAMED("thrift"),
  HTTP("http"),
  HTTP_2("h2"),
  RSOCKET("rs");

  final String protocol;

  ThriftTransportType(String protocol) {
    this.protocol = protocol;
  }

  public String protocol() {
    return protocol;
  }

  public static ThriftTransportType fromProtocol(String protocol) {
    switch (protocol) {
      case "header":
        return HEADER;
      case "framed":
        return FRAMED;
      case "HTTP":
        return HTTP;
      case "HTTP_2":
        return HTTP_2;
      case "rs":
        return RSOCKET;
      default:
        throw new IllegalArgumentException("unknown protocol " + protocol);
    }
  }
}
