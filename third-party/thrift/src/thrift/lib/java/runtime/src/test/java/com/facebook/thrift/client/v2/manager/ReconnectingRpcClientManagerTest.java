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

package com.facebook.thrift.client.v2.manager;

import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcClientFactory;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Mono;

public class ReconnectingRpcClientManagerTest {
  private static final SocketAddress ADDRESS =
      InetSocketAddress.createUnresolved("localhost", 8080);

  @Test
  public void testDisposeDuringRetryBackoffFailsAcquirePromptly() throws Exception {
    CountDownLatch firstFailure = new CountDownLatch(1);
    RpcClientFactory factory =
        __ ->
            Mono.defer(
                () ->
                    Mono.<RpcClient>error(new RuntimeException("connect failed"))
                        .doOnError(___ -> firstFailure.countDown()));

    ReconnectingRpcClientManager manager = new ReconnectingRpcClientManager(factory, ADDRESS);

    CompletableFuture<RpcClient> acquireFuture = manager.acquire().toFuture();

    assertTrue(firstFailure.await(5, TimeUnit.SECONDS));

    // Dispose while retryWhen is sleeping in backoff, not before the first failure is observed.
    Thread.sleep(10);
    manager.dispose();

    ExecutionException error =
        assertThrows(ExecutionException.class, () -> acquireFuture.get(30, TimeUnit.MILLISECONDS));
    assertInstanceOf(IllegalStateException.class, error.getCause());
  }
}
