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

package com.facebook.thrift.rsocket.server;

import com.facebook.thrift.model.StreamResponse;
import com.facebook.thrift.test.rocket.InitialTestResponse;
import com.facebook.thrift.test.rocket.TestAnnotatedMessageException;
import com.facebook.thrift.test.rocket.TestException;
import com.facebook.thrift.test.rocket.TestFunctionException;
import com.facebook.thrift.test.rocket.TestMessageException;
import com.facebook.thrift.test.rocket.TestRequest;
import com.facebook.thrift.test.rocket.TestRequest2;
import com.facebook.thrift.test.rocket.TestResponse;
import com.facebook.thrift.test.rocket.TestService;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public class TestServiceHandler implements TestService.Reactive {

  public static TestRequest inputParameter;

  @Override
  public Flux<TestResponse> streamDeclaredExceptionMultiInput(
      TestRequest testRequest, TestRequest2 testRequest2) {
    return Flux.just(
        new TestResponse.Builder()
            .setIntField(testRequest.getIntField() + testRequest2.getShortField())
            .setStrField(
                testRequest.getStrField()
                    + testRequest2.getStrField()
                    + testRequest2.getShortField())
            .build());
  }

  @Override
  public Mono<Void> requestResponseVoid(TestRequest testRequest) {
    inputParameter = testRequest;
    return Mono.empty();
  }

  @Override
  public Mono<TestResponse> requestResponse(TestRequest testRequest) {
    return Mono.just(
        new TestResponse.Builder()
            .setIntField(testRequest.getIntField())
            .setStrField(testRequest.getStrField())
            .build());
  }

  @Override
  public Flux<TestResponse> streamResponse(TestRequest testRequest) {
    return Flux.range(0, 10)
        .map(
            i ->
                new TestResponse.Builder()
                    .setIntField(testRequest.getIntField() + i)
                    .setStrField(testRequest.getStrField() + i)
                    .build());
  }

  @Override
  public Flux<TestResponse> streamResponseNull(TestRequest testRequest) {
    return null;
  }

  @Override
  public Flux<TestResponse> streamResponseEmpty(TestRequest testRequest) {
    return Flux.empty();
  }

  @Override
  public Flux<StreamResponse<InitialTestResponse, TestResponse>> streamInitialResponse(
      TestRequest testRequest) {
    return Flux.range(-1, 11)
        .map(
            i -> {
              if (i == -1) {
                return StreamResponse.fromFirstResponse(
                    new InitialTestResponse.Builder().setIntField(100).build());
              } else {
                return StreamResponse.fromData(
                    new TestResponse.Builder()
                        .setIntField(testRequest.getIntField() + i)
                        .setStrField(testRequest.getStrField() + i)
                        .build());
              }
            });
  }

  @Override
  public Flux<TestResponse> streamDeclaredException(TestRequest testRequest) {
    return Flux.error(new TestException("exc"));
  }

  @Override
  public Flux<TestResponse> streamDeclaredException2(TestRequest testRequest) {
    throw new TestException("exc");
  }

  @Override
  public Flux<TestResponse> streamDeclaredMessageFieldException(TestRequest testRequest) {
    return Flux.error(new TestMessageException("exc"));
  }

  @Override
  public Flux<TestResponse> streamDeclaredAnnotatedMessageException(TestRequest testRequest) {
    return Flux.error(new TestAnnotatedMessageException("exc"));
  }

  @Override
  public Flux<TestResponse> streamDeclaredAndFunctionException(TestRequest testRequest) {
    return Flux.error(new TestFunctionException("exc"));
  }

  @Override
  public Flux<TestResponse> streamUndeclaredException(TestRequest testRequest) {
    return Flux.error(new IllegalArgumentException("exc"));
  }

  @Override
  public Flux<TestResponse> streamUndeclaredException2(TestRequest testRequest) {
    throw new IllegalArgumentException("exc");
  }

  @Override
  public Flux<TestResponse> streamUndeclaredAndFunctionException(TestRequest testRequest) {
    return Flux.error(new IllegalArgumentException("exc"));
  }

  @Override
  public Flux<StreamResponse<InitialTestResponse, TestResponse>>
      streamInitialResponseDeclaredAndFunctionException(TestRequest testRequest) {
    return Flux.error(new TestFunctionException("exc"));
  }

  @Override
  public Flux<StreamResponse<InitialTestResponse, TestResponse>>
      streamInitialResponseDeclaredException(TestRequest testRequest) {
    return Flux.error(new TestException("exc"));
  }

  @Override
  public Flux<TestResponse> streamDeclaredExceptionFluxError(TestRequest testRequest) {
    Flux<TestResponse> flux =
        Flux.range(0, 4)
            .map(
                i ->
                    new TestResponse.Builder()
                        .setIntField(testRequest.getIntField() + i)
                        .setStrField(testRequest.getStrField() + i)
                        .build());

    return flux.concatWith(Flux.error(new TestException("exc")));
  }

  @Override
  public void dispose() {}

  @Override
  public Flux<StreamResponse<InitialTestResponse, TestResponse>>
      streamInitialResponseDeclaredExceptionAfterFirst(TestRequest testRequest) {
    Flux<StreamResponse<InitialTestResponse, TestResponse>> first =
        Flux.just(
            StreamResponse.fromFirstResponse(
                new InitialTestResponse.Builder().setIntField(100).build()));
    return first.concatWith(Flux.error(new TestException("exc")));
  }

  @Override
  public Flux<TestResponse> streamFunctionExceptionSyncThrow(TestRequest testRequest) {
    throw new TestFunctionException("exc");
  }

  @Override
  public Flux<StreamResponse<InitialTestResponse, TestResponse>>
      streamInitialResponseInvalidFirstResponse(TestRequest testRequest) {
    return Flux.just(createInvalidFirstResponse());
  }

  @SuppressWarnings("unchecked")
  private static StreamResponse<InitialTestResponse, TestResponse> createInvalidFirstResponse() {
    try {
      Constructor<StreamResponse> constructor = StreamResponse.class.getDeclaredConstructor();
      constructor.setAccessible(true);
      StreamResponse<InitialTestResponse, TestResponse> response = constructor.newInstance();

      Field isFirstResponseField = StreamResponse.class.getDeclaredField("isFirstResponse");
      isFirstResponseField.setAccessible(true);
      isFirstResponseField.setBoolean(response, true);
      return response;
    } catch (ReflectiveOperationException e) {
      throw new AssertionError("Failed to create invalid first response", e);
    }
  }
}
