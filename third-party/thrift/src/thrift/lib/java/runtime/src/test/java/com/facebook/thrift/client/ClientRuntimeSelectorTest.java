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

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTrue;

import org.junit.jupiter.api.Test;
import reactor.core.CoreSubscriber;
import reactor.core.Disposable;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;
import reactor.test.StepVerifier;

public class ClientRuntimeSelectorTest {

  @Test
  public void testResolveHonorsExplicitConfigOverride() {
    assertEquals(
        ClientRuntimeMode.LEGACY,
        ClientRuntimeSelector.resolve(
            new ThriftClientConfig().setClientRuntimeMode(ClientRuntimeMode.LEGACY)));
    assertEquals(
        ClientRuntimeMode.V2,
        ClientRuntimeSelector.resolve(
            new ThriftClientConfig().setClientRuntimeMode(ClientRuntimeMode.V2)));
  }

  @Test
  public void testLegacySourceKeepsDisposeNoop() {
    TestRpcClient rpcClient = new TestRpcClient();
    RpcClientSource source =
        ClientRuntimeSelector.createSource(Mono.just(rpcClient), ClientRuntimeMode.LEGACY);

    assertSame(rpcClient, source.acquire().block());

    source.dispose();

    assertFalse(source.isDisposed());
    assertFalse(rpcClient.isDisposed());
    assertSame(rpcClient, source.acquire().block());
  }

  @Test
  public void testV2SourceOwnsAcquiredClientAndRejectsAfterDispose() {
    TestRpcClient rpcClient = new TestRpcClient();
    RpcClientSource source =
        ClientRuntimeSelector.createSource(Mono.just(rpcClient), ClientRuntimeMode.V2);

    assertSame(rpcClient, source.acquire().block());

    source.dispose();

    assertTrue(source.isDisposed());
    assertTrue(rpcClient.isDisposed());
    StepVerifier.create(source.acquire()).expectError(IllegalStateException.class).verify();
  }

  @Test
  public void testV2SourceDoesNotDisposeDisposableSourceMono() {
    TestRpcClient rpcClient = new TestRpcClient();
    TestDisposableRpcClientMono rpcClientMono = new TestDisposableRpcClientMono(rpcClient);
    RpcClientSource source =
        ClientRuntimeSelector.createSource(rpcClientMono, ClientRuntimeMode.V2);

    assertSame(rpcClient, source.acquire().block());

    source.dispose();

    assertTrue(source.isDisposed());
    assertFalse(rpcClientMono.isDisposed());
    assertTrue(rpcClient.isDisposed());
    StepVerifier.create(source.acquire()).expectError(IllegalStateException.class).verify();
  }

  private static final class TestRpcClient implements RpcClient {
    private final Sinks.One<Void> onClose = Sinks.one();
    private boolean disposed;

    @Override
    public Mono<Void> onClose() {
      return onClose.asMono();
    }

    @Override
    public void dispose() {
      if (!disposed) {
        disposed = true;
        onClose.tryEmitEmpty();
      }
    }

    @Override
    public boolean isDisposed() {
      return disposed;
    }
  }

  private static final class TestDisposableRpcClientMono extends Mono<RpcClient>
      implements Disposable {
    private final Mono<RpcClient> delegate;
    private boolean disposed;

    private TestDisposableRpcClientMono(RpcClient rpcClient) {
      this.delegate = Mono.just(rpcClient);
    }

    @Override
    public void subscribe(CoreSubscriber<? super RpcClient> actual) {
      delegate.subscribe(actual);
    }

    @Override
    public void dispose() {
      disposed = true;
    }

    @Override
    public boolean isDisposed() {
      return disposed;
    }
  }
}
