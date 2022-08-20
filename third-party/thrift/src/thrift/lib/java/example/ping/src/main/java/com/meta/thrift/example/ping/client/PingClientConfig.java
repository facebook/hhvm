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

package com.meta.thrift.example.ping.client;

public class PingClientConfig {
  private static final String DEFAULT_HOST = "localhost";
  private static final int DEFAULT_PORT = 7777;
  private static final String DEFAULT_TRANSPORT = "header";
  private static final String DEFAULT_METHOD = "ping";

  private final String host;
  private final int port;
  private final String transport;
  private final String method;

  public PingClientConfig(String host, int port, String transport, String method) {
    this.host = host;
    this.port = port;
    this.transport = transport;
    this.method = method;
  }

  public String getHost() {
    return host;
  }

  public int getPort() {
    return port;
  }

  public String getTransport() {
    return transport;
  }

  public String getMethod() {
    return method;
  }

  public static class Builder {
    private String host = DEFAULT_HOST;
    private int port = DEFAULT_PORT;
    private String transport = DEFAULT_TRANSPORT;
    private String method = DEFAULT_METHOD;

    public void setHost(String host) {
      this.host = host;
    }

    public void setPort(int port) {
      this.port = port;
    }

    public void setTransport(String transport) {
      this.transport = transport;
    }

    public void setMethod(String method) {
      this.method = method;
    }

    public PingClientConfig build() {
      return new PingClientConfig(host, port, transport, method);
    }
  }
}
