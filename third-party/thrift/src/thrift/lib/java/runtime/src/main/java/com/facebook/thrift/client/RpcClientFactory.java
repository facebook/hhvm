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

package com.facebook.thrift.client;

import com.facebook.swift.service.ThriftClientEventHandler;
import com.facebook.swift.service.ThriftClientStats;
import com.facebook.thrift.client.v2.manager.RpcClientBinding;
import com.facebook.thrift.client.v2.transport.RpcClientFactoryV2;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.base.Preconditions;
import java.net.SocketAddress;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * Higher-level binding factory for typed Thrift clients.
 *
 * <p>Produces {@link RpcClientBinding} handles that own the manager-backed lifecycle for a typed
 * client. For the lower-level raw-transport contract see {@link RpcClientTransportFactory}.
 */
public interface RpcClientFactory {
  /**
   * Returns an OWNED {@link RpcClientBinding} for the given address. Closing the typed client
   * disposes the underlying manager and its transports.
   */
  RpcClientBinding createRpcClientBinding(SocketAddress socketAddress);

  /**
   * Builder to create an RpcClientFactory. By default it creates an RpcClientFactory with RSocket
   * disabled, stats enabled, reconnecting client enabled, and simple load balancing enabled.
   */
  final class Builder {
    private boolean disableRSocket = true;
    private boolean disableStats = false;
    private boolean disableReconnectingClient = false;
    private boolean disableTimeout = false;
    private Map<String, String> headerTokens;
    private List<ThriftClientEventHandler> clientEventHandlers;
    private int connectionPoolSize = RpcResources.getNumEventLoopThreads();
    private boolean handleHeaderResponse = false;

    private ThriftClientConfig thriftClientConfig;
    private ThriftClientStats thriftClientStats = ThriftClientStatsHolder.getThriftClientStats();

    private Builder() {}

    public Builder setDisableRSocket(boolean disableRSocket) {
      this.disableRSocket = disableRSocket;
      return this;
    }

    public Builder setEnableHandleHeaderResponse(boolean handleHeaderResponse) {
      this.handleHeaderResponse = handleHeaderResponse;
      return this;
    }

    public Builder setDisableStats(boolean disableStats) {
      this.disableStats = disableStats;
      return this;
    }

    public Builder setDisableReconnectingClient(boolean disableReconnectingClient) {
      this.disableReconnectingClient = disableReconnectingClient;
      return this;
    }

    public Builder setDisableLoadBalancing(boolean disableLoadBalancing) {
      if (disableLoadBalancing) {
        this.connectionPoolSize = 1;
      }
      return this;
    }

    public Builder setDisableTimeout(boolean disableTimeout) {
      this.disableTimeout = disableTimeout;
      return this;
    }

    public Builder setHeaderTokens(Map<String, String> headerTokens) {
      this.headerTokens = headerTokens;
      return this;
    }

    public Builder setClientEventHandlers(List<ThriftClientEventHandler> clientEventHandlers) {
      this.clientEventHandlers = clientEventHandlers;
      return this;
    }

    public Builder setConnectionPoolSize(int poolSize) {
      Preconditions.checkArgument(
          poolSize >= 1, "0 or negative connection pool size is not allowed");
      this.connectionPoolSize = poolSize;
      return this;
    }

    public Builder setThriftClientConfig(ThriftClientConfig thriftClientConfig) {
      this.thriftClientConfig = thriftClientConfig;
      return this;
    }

    public Builder setThriftClientStats(ThriftClientStats thriftClientStats) {
      this.thriftClientStats = thriftClientStats;
      return this;
    }

    public RpcClientFactory build() {
      Objects.requireNonNull(thriftClientConfig, "ThriftClientConfig is required");
      Objects.requireNonNull(thriftClientStats, "thriftClientStats is required");
      return RpcClientFactoryV2.builder()
          .setDisableRSocket(disableRSocket)
          .setEnableHandleHeaderResponse(handleHeaderResponse)
          .setDisableStats(disableStats)
          .setDisableReconnectingClient(disableReconnectingClient)
          .setDisableTimeout(disableTimeout)
          .setHeaderTokens(headerTokens)
          .setClientEventHandlers(clientEventHandlers)
          .setConnectionPoolSize(connectionPoolSize)
          .setThriftClientConfig(thriftClientConfig)
          .setThriftClientStats(thriftClientStats)
          .build();
    }
  }

  static Builder builder() {
    return new Builder();
  }
}
