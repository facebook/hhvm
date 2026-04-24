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

package com.facebook.thrift.client.v2.transport;

import com.facebook.swift.service.ThriftClientEventHandler;
import com.facebook.swift.service.ThriftClientStats;
import com.facebook.thrift.client.CachedRpcClientFactory;
import com.facebook.thrift.client.ClientBuilder;
import com.facebook.thrift.client.ClientRuntimeSelector;
import com.facebook.thrift.client.EventHandlerRpcClientFactory;
import com.facebook.thrift.client.InstrumentedRpcClientFactory;
import com.facebook.thrift.client.ReconnectingRpcClientFactory;
import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.RpcClientSource;
import com.facebook.thrift.client.SimpleLoadBalancingRpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.client.ThriftClientStatsHolder;
import com.facebook.thrift.client.TimeoutRpcClientFactory;
import com.facebook.thrift.client.TokenPassingRpcClientFactory;
import com.facebook.thrift.client.v2.manager.ReconnectingRpcClientManagerFactory;
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
import reactor.core.publisher.Mono;

/**
 * V2 transport/factory entrypoint.
 *
 * <p>The legacy {@link Mono}-returning surface is kept for compatibility, while typed-client
 * construction should use {@link #createRpcClientSource(SocketAddress)} to enter the manager-backed
 * v2 lifecycle stack.
 */
public final class RpcClientFactoryV2 implements RpcClientFactory {
  private final RpcClientFactory legacyMonoFactory;
  private final RpcClientManagerFactory managerFactory;

  private RpcClientFactoryV2(
      RpcClientFactory legacyMonoFactory, RpcClientManagerFactory managerFactory) {
    this.legacyMonoFactory = Objects.requireNonNull(legacyMonoFactory);
    this.managerFactory = Objects.requireNonNull(managerFactory);
  }

  /**
   * Legacy entrypoint preserved for callers that still use raw {@code Mono<RpcClient>}. This
   * delegates to the legacy mono factory chain and does NOT enter the v2 manager stack.
   */
  @Override
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {
    return legacyMonoFactory.createRpcClient(socketAddress);
  }

  /**
   * V2 entrypoint. Creates a manager-backed {@link RpcClientSource} with OWNED semantics. This is
   * the path that {@link ClientBuilder#build(RpcClientFactory, SocketAddress)} uses when the
   * runtime is set to V2.
   */
  @Override
  public RpcClientSource createRpcClientSource(SocketAddress socketAddress) {
    return ClientRuntimeSelector.createOwnedSource(
        managerFactory.createRpcClientManager(socketAddress));
  }

  /**
   * Builder for the manager-backed v2 runtime.
   *
   * <p>The knobs intentionally mirror {@link RpcClientFactory.Builder} so callers can cut over
   * without learning a new configuration model.
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
    private boolean cacheClient = true;

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

    public Builder setCacheClient(boolean cacheClient) {
      this.cacheClient = cacheClient;
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

    public RpcClientManagerFactory buildManagerFactory() {
      validate();
      recordTransport();
      return buildManagerFactoryInternal();
    }

    public RpcClientFactoryV2 build() {
      validate();
      recordTransport();
      return new RpcClientFactoryV2(buildLegacyMonoFactory(), buildManagerFactoryInternal());
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
     * Builds the v2 manager factory chain: transport factory (raw connections + per-request
     * decoration) wrapped in reconnecting or single managers, optionally load-balanced across N
     * slots. CachedRpcClientFactory is intentionally omitted because SingleRpcClientManager handles
     * connection caching internally.
     */
    private RpcClientManagerFactory buildManagerFactoryInternal() {
      RpcClientFactory transportFactory = buildManagerTransportFactory();
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
     * Builds the full legacy decorator chain for the {@link #createRpcClient} compatibility path.
     * This is the same chain that {@link RpcClientFactory.Builder#buildLegacyFactory()} produces.
     */
    private RpcClientFactory buildLegacyMonoFactory() {
      RpcClientFactory rpcClientFactory;
      if (disableRSocket) {
        if (handleHeaderResponse) {
          throw new IllegalArgumentException(
              "handleHeaderResponse is only applicable if using RSocket");
        }
        rpcClientFactory = new LegacyRpcClientFactory(thriftClientConfig);
      } else {
        if (handleHeaderResponse) {
          rpcClientFactory = new HeaderAwareRpcClientFactory(thriftClientConfig);
        } else {
          rpcClientFactory = new RSocketRpcClientFactory(thriftClientConfig);
        }
        if (cacheClient) {
          rpcClientFactory = new CachedRpcClientFactory(rpcClientFactory);
        }
      }

      if (!disableStats) {
        rpcClientFactory = new InstrumentedRpcClientFactory(rpcClientFactory, thriftClientStats);
      }

      if (headerTokens != null && !headerTokens.isEmpty()) {
        rpcClientFactory = new TokenPassingRpcClientFactory(rpcClientFactory, headerTokens);
      }

      if (clientEventHandlers != null && !clientEventHandlers.isEmpty()) {
        rpcClientFactory = new EventHandlerRpcClientFactory(rpcClientFactory, clientEventHandlers);
      }

      if (!disableReconnectingClient) {
        rpcClientFactory = new ReconnectingRpcClientFactory(rpcClientFactory);
      }

      if (!disableTimeout) {
        rpcClientFactory = new TimeoutRpcClientFactory(rpcClientFactory, thriftClientConfig);
      }

      if (connectionPoolSize >= 1) {
        rpcClientFactory =
            new SimpleLoadBalancingRpcClientFactory(rpcClientFactory, connectionPoolSize);
      }

      return rpcClientFactory;
    }

    /**
     * Builds the transport-only factory for the v2 manager path. This includes per-connection
     * decoration (stats, tokens, event handlers, timeout) but excludes lifecycle concerns
     * (reconnecting, caching, load-balancing) which are handled by the manager layer above.
     */
    private RpcClientFactory buildManagerTransportFactory() {
      RpcClientFactory transportFactory;
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
