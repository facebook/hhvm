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

package com.facebook.swift.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;

import com.facebook.nifty.core.RequestContext;
import com.facebook.thrift.util.NettyNiftyRequestContext;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.concurrent.atomic.AtomicInteger;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TException;
import org.junit.jupiter.api.Test;

/** Locks down the {@link ContextChain#preprocess} contract for admission-control handlers. */
public class ContextChainPreprocessTest {

  private static final RequestContext REQ_CTX = new NettyNiftyRequestContext(new HashMap<>(), null);

  @Test
  public void noHandlers_isNoOp() throws TException {
    ContextChain chain = new ContextChain(Collections.emptyList(), "Svc.method", REQ_CTX);
    chain.preprocess();
  }

  @Test
  public void allHandlersAccept_eachRunsInOrderAndReturns() throws TException {
    AtomicInteger counter = new AtomicInteger();
    int[] order = new int[3];
    ContextChain chain =
        new ContextChain(
            Arrays.asList(
                handler(c -> order[0] = counter.incrementAndGet()),
                handler(c -> order[1] = counter.incrementAndGet()),
                handler(c -> order[2] = counter.incrementAndGet())),
            "Svc.method",
            REQ_CTX);
    chain.preprocess();
    assertThat(order).containsExactly(1, 2, 3);
  }

  @Test
  public void handlerThrowsTApplicationException_propagatesAndShortCircuits() {
    AtomicInteger laterHandlerCalls = new AtomicInteger();
    TApplicationException expected =
        new TApplicationException(TApplicationException.LOADSHEDDING, "shed me");

    ContextChain chain =
        new ContextChain(
            Arrays.asList(
                handler(
                    c -> {
                      throw expected;
                    }),
                handler(c -> laterHandlerCalls.incrementAndGet())),
            "Svc.method",
            REQ_CTX);

    assertThatThrownBy(chain::preprocess).isSameAs(expected);
    // A shed MUST stop subsequent handlers so cost stays bounded to the first-matching gate.
    assertThat(laterHandlerCalls.get()).isZero();
  }

  @Test
  public void handlerThrowsGenericTException_propagates() {
    TException expected = new TException("nope");
    ContextChain chain =
        new ContextChain(
            Collections.singletonList(
                handler(
                    c -> {
                      throw expected;
                    })),
            "Svc.method",
            REQ_CTX);
    assertThatThrownBy(chain::preprocess).isSameAs(expected);
  }

  @Test
  public void handlerThrowsRuntimeException_propagates() {
    RuntimeException expected = new RuntimeException("bad");
    ContextChain chain =
        new ContextChain(
            Collections.singletonList(
                handler(
                    c -> {
                      throw expected;
                    })),
            "Svc.method",
            REQ_CTX);
    assertThatThrownBy(chain::preprocess).isSameAs(expected);
  }

  /** The per-handler context returned by {@code getContext} flows through to {@code preprocess}. */
  @Test
  public void getContextResultIsPassedToPreprocess() throws TException {
    Object marker = new Object();
    Object[] receivedInPreprocess = new Object[1];
    String[] receivedMethodName = new String[1];

    ThriftEventHandler h =
        new ThriftEventHandler() {
          @Override
          public Object getContext(String methodName, RequestContext requestContext) {
            return marker;
          }

          @Override
          public void preprocess(Object context, String methodName) {
            receivedInPreprocess[0] = context;
            receivedMethodName[0] = methodName;
          }
        };

    ContextChain chain = new ContextChain(Collections.singletonList(h), "Svc.method", REQ_CTX);
    chain.preprocess();

    assertThat(receivedInPreprocess[0]).isSameAs(marker);
    assertThat(receivedMethodName[0]).isEqualTo("Svc.method");
  }

  /**
   * If a later handler's {@code getContext} throws, the constructor MUST call {@code done()} on
   * every prior handler whose {@code getContext} succeeded -- otherwise an admission counter (e.g.
   * {@code LoadSheddingHandler}'s active-request count) increments without ever decrementing.
   */
  @Test
  public void constructorPartialFailure_unwindsPriorHandlers() {
    AtomicInteger first = new AtomicInteger();
    AtomicInteger third = new AtomicInteger();
    ThriftEventHandler h1 = countingDone(first);
    ThriftEventHandler h2 =
        new ThriftEventHandler() {
          @Override
          public Object getContext(String methodName, RequestContext requestContext) {
            throw new RuntimeException("h2 getContext crash");
          }
        };
    ThriftEventHandler h3 = countingDone(third);

    assertThatThrownBy(() -> new ContextChain(Arrays.asList(h1, h2, h3), "Svc.method", REQ_CTX))
        .isInstanceOf(RuntimeException.class)
        .hasMessage("h2 getContext crash");

    assertThat(first.get()).as("h1.done MUST fire (its getContext succeeded)").isEqualTo(1);
    assertThat(third.get()).as("h3.done MUST NOT fire (its getContext never ran)").isZero();
  }

  /** Unwind reverses construction order so destructor-style cleanup is symmetric with setup. */
  @Test
  public void constructorPartialFailure_unwindsInReverseOrder() {
    int[] order = new int[2];
    AtomicInteger counter = new AtomicInteger();
    ThriftEventHandler h1 =
        new ThriftEventHandler() {
          @Override
          public void done(Object c, String m) {
            order[0] = counter.incrementAndGet();
          }
        };
    ThriftEventHandler h2 =
        new ThriftEventHandler() {
          @Override
          public void done(Object c, String m) {
            order[1] = counter.incrementAndGet();
          }
        };
    ThriftEventHandler crash =
        new ThriftEventHandler() {
          @Override
          public Object getContext(String methodName, RequestContext requestContext) {
            throw new RuntimeException("crash");
          }
        };

    assertThatThrownBy(() -> new ContextChain(Arrays.asList(h1, h2, crash), "Svc.method", REQ_CTX))
        .isInstanceOf(RuntimeException.class);

    // h2 (the most recently constructed) is torn down first, then h1.
    assertThat(order[1]).as("h2.done fires first").isEqualTo(1);
    assertThat(order[0]).as("h1.done fires second").isEqualTo(2);
  }

  private static ThriftEventHandler countingDone(AtomicInteger counter) {
    return new ThriftEventHandler() {
      @Override
      public void done(Object c, String m) {
        counter.incrementAndGet();
      }
    };
  }

  @FunctionalInterface
  private interface PreprocessBody {
    void run(Object context) throws TException;
  }

  private static ThriftEventHandler handler(PreprocessBody body) {
    return new ThriftEventHandler() {
      @Override
      public void preprocess(Object context, String methodName) throws TException {
        body.run(context);
      }
    };
  }
}
