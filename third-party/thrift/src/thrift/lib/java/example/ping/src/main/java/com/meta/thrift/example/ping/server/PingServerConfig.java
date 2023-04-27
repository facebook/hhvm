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

package com.meta.thrift.example.ping.server;

public class PingServerConfig {
  private static final int DEFAULT_PORT = 7777;
  private static final String DEFAULT_TRANSPORT = "header";

  private final int port;
  private final String transport;

  public PingServerConfig(int port, String transport) {
    this.port = port;
    this.transport = transport;
  }

  public int getPort() {
    return port;
  }

  public String getTransport() {
    return transport;
  }

  public static class Builder {
    private int port = DEFAULT_PORT;
    private String transport = DEFAULT_TRANSPORT;

    public void setPort(int port) {
      this.port = port;
    }

    public void setTransport(String transport) {
      this.transport = transport;
    }

    public PingServerConfig build() {
      return new PingServerConfig(port, transport);
    }
  }
}
