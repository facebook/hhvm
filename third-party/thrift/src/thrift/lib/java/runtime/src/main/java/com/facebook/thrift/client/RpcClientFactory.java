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
import com.facebook.thrift.client.v2.manager.ClientOwnership;
import com.facebook.thrift.client.v2.manager.ReconnectingRpcClientManagerFactory;
import com.facebook.thrift.client.v2.manager.RpcClientBinding;
import com.facebook.thrift.client.v2.manager.RpcClientManagerFactory;
import com.facebook.thrift.client.v2.manager.SimpleLoadBalancingRpcClientManagerFactory;
import com.facebook.thrift.client.v2.manager.SingleRpcClientManagerFactory;
import com.facebook.thrift.legacy.client.LegacyRpcClientFactory;
import com.facebook.thrift.metadata.ClientInfo;
import com.facebook.thrift.rsocket.client.HeaderAwareRpcClientFactory;
import com.facebook.thrift.rsocket.client.RSocketRpcClientFactory;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.base.Preconditions;
import java.net.SocketAddress;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * Binding factory for typed Thrift clients.
 *
 * <p>Produces manager-backed {@link RpcClientBinding} handles for typed clients via {@link
 * #createRpcClientBinding(SocketAddress)}. The lower-level raw-transport contract is {@link
 * RpcClientTransportFactory}; the {@link Builder} composes transport factories internally.
 */
public final class RpcClientFactory {
  private final RpcClientManagerFactory managerFactory;

  private RpcClientFactory(RpcClientManagerFactory managerFactory) {
    this.managerFactory = Objects.requireNonNull(managerFactory);
  }

  /**
   * Creates a manager-backed {@link RpcClientBinding} with OWNED semantics. This is the path that
   * {@link ClientBuilder#build(RpcClientFactory, SocketAddress)} uses. Closing the typed client
   * disposes the manager and its underlying transports.
   */
  public RpcClientBinding createRpcClientBinding(SocketAddress socketAddress) {
    return new RpcClientBinding(
        managerFactory.createRpcClientManager(socketAddress), ClientOwnership.OWNED);
  }

  /**
   * Builder for the manager-backed runtime. Defaults: RSocket disabled, stats enabled, reconnecting
   * client enabled, single-slot connection pool (no load balancing), timeout enabled.
   */
  public static final class Builder {
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

    /**
     * Builds the manager factory chain directly, skipping the binding-factory wrapper. Used by SR
     * proxy-client code that composes its own manager pipeline on top of the v2 transport stack.
     */
    public RpcClientManagerFactory buildManagerFactory() {
      validate();
      recordTransport();
      return buildManagerFactoryInternal();
    }

    public RpcClientFactory build() {
      validate();
      recordTransport();
      return new RpcClientFactory(buildManagerFactoryInternal());
    }

    private void validate() {
      Objects.requireNonNull(thriftClientConfig, "ThriftClientConfig is required");
      Objects.requireNonNull(thriftClientStats, "thriftClientStats is required");
    }

    private void recordTransport() {
      if (disableRSocket || handleHeaderResponse) {
        ClientInfo.addTransport(ClientInfo.Transport.HEADER);
      } else {
        ClientInfo.addTransport(ClientInfo.Transport.ROCKET);
      }
    }

    /**
     * Builds the manager factory chain: transport factory (raw connections + per-request
     * decoration) wrapped in reconnecting or single managers, optionally load-balanced across N
     * slots. Connection caching is handled by {@code SingleRpcClientManager} internally.
     */
    private RpcClientManagerFactory buildManagerFactoryInternal() {
      RpcClientTransportFactory transportFactory = buildManagerTransportFactory();
      RpcClientManagerFactory managerFactory =
          disableReconnectingClient
              ? new SingleRpcClientManagerFactory(transportFactory)
              : new ReconnectingRpcClientManagerFactory(transportFactory);

      if (connectionPoolSize > 1) {
        managerFactory =
            new SimpleLoadBalancingRpcClientManagerFactory(managerFactory, connectionPoolSize);
      }

      return managerFactory;
    }

    /**
     * Builds the transport-only factory. This includes per-connection decoration (stats, tokens,
     * event handlers, timeout) but excludes lifecycle concerns (reconnecting, caching,
     * load-balancing) which are handled by the manager layer above.
     */
    private RpcClientTransportFactory buildManagerTransportFactory() {
      RpcClientTransportFactory transportFactory;
      if (disableRSocket) {
        if (handleHeaderResponse) {
          throw new IllegalArgumentException(
              "handleHeaderResponse is only applicable if using RSocket");
        }
        transportFactory = new LegacyRpcClientFactory(thriftClientConfig);
      } else {
        if (handleHeaderResponse) {
          transportFactory = new HeaderAwareRpcClientFactory(thriftClientConfig);
        } else {
          transportFactory = new RSocketRpcClientFactory(thriftClientConfig);
        }
      }

      if (!disableStats) {
        transportFactory = new InstrumentedRpcClientFactory(transportFactory, thriftClientStats);
      }

      if (headerTokens != null && !headerTokens.isEmpty()) {
        transportFactory = new TokenPassingRpcClientFactory(transportFactory, headerTokens);
      }

      if (clientEventHandlers != null && !clientEventHandlers.isEmpty()) {
        transportFactory = new EventHandlerRpcClientFactory(transportFactory, clientEventHandlers);
      }

      if (!disableTimeout) {
        transportFactory = new TimeoutRpcClientFactory(transportFactory, thriftClientConfig);
      }

      return transportFactory;
    }
  }

  public static Builder builder() {
    return new Builder();
  }
}
