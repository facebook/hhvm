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

package com.meta.thrift.conformance.rpc.server;

import com.facebook.thrift.client.ResponseWrapper;
import com.facebook.thrift.client.RpcOptions;
import com.facebook.thrift.model.StreamResponse;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;
import org.apache.thrift.conformance.ClientTestResult;
import org.apache.thrift.conformance.RPCConformanceService;
import org.apache.thrift.conformance.Request;
import org.apache.thrift.conformance.RequestResponseBasicServerTestResult;
import org.apache.thrift.conformance.RequestResponseDeclaredExceptionServerTestResult;
import org.apache.thrift.conformance.RequestResponseNoArgVoidResponseServerTestResult;
import org.apache.thrift.conformance.RequestResponseUndeclaredExceptionServerTestResult;
import org.apache.thrift.conformance.Response;
import org.apache.thrift.conformance.RpcTestCase;
import org.apache.thrift.conformance.ServerInstruction;
import org.apache.thrift.conformance.ServerTestResult;
import org.apache.thrift.conformance.StreamBasicServerTestResult;
import org.apache.thrift.conformance.StreamDeclaredExceptionServerTestResult;
import org.apache.thrift.conformance.StreamInitialDeclaredExceptionServerTestResult;
import org.apache.thrift.conformance.StreamInitialResponseServerTestResult;
import org.apache.thrift.conformance.StreamInitialUndeclaredExceptionServerTestResult;
import org.apache.thrift.conformance.StreamUndeclaredExceptionServerTestResult;
import org.apache.thrift.conformance.UserException;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public class RpcServerReactiveHandler implements RPCConformanceService.Reactive {

  private ServerInstruction instruction;
  private ServerTestResult testResult;

  @Override
  public Mono<Void> sendTestCase(RpcTestCase rpcTestCase) {
    instruction = rpcTestCase.getServerInstruction();
    return Mono.empty();
  }

  @Override
  public Mono<ServerTestResult> getTestResult() {
    return Mono.just(testResult);
  }

  @Override
  public Mono<RpcTestCase> getTestCase() {
    throw new RuntimeException("Not implemented, needed for client testing only");
  }

  @Override
  public Mono<Void> sendTestResult(ClientTestResult clientTestResult) {
    throw new RuntimeException("Not implemented, needed for client testing only");
  }

  @Override
  public Mono<Response> requestResponseBasic(Request request) {
    testResult =
        ServerTestResult.fromRequestResponseBasic(
            new RequestResponseBasicServerTestResult.Builder().setRequest(request).build());
    return Mono.just(instruction.getRequestResponseBasic().getResponse());
  }

  @Override
  public Mono<Response> requestResponseBasic(Request request, RpcOptions rpcOptions) {
    testResult =
        ServerTestResult.fromRequestResponseBasic(
            new RequestResponseBasicServerTestResult.Builder().setRequest(request).build());
    return Mono.just(instruction.getRequestResponseBasic().getResponse());
  }

  @Override
  public Mono<Void> requestResponseDeclaredException(Request request) {
    testResult =
        ServerTestResult.fromRequestResponseDeclaredException(
            new RequestResponseDeclaredExceptionServerTestResult.Builder()
                .setRequest(request)
                .build());
    return Mono.error(instruction.getRequestResponseDeclaredException().getUserException());
  }

  @Override
  public Mono<Void> requestResponseUndeclaredException(Request request) {
    testResult =
        ServerTestResult.fromRequestResponseUndeclaredException(
            new RequestResponseUndeclaredExceptionServerTestResult.Builder()
                .setRequest(request)
                .build());
    return Mono.error(
        new RuntimeException(
            instruction.getRequestResponseUndeclaredException().getExceptionMessage()));
  }

  @Override
  public Mono<Void> requestResponseNoArgVoidResponse() {
    testResult =
        ServerTestResult.fromRequestResponseNoArgVoidResponse(
            new RequestResponseNoArgVoidResponseServerTestResult.Builder().build());
    return Mono.empty();
  }

  @Override
  public Mono<Response> requestResponseTimeout(Request request) {
    return Mono.empty();
  }

  @Override
  public Mono<Void> basicInteractionFactoryFunction(int i) {
    return null;
  }

  @Override
  public Flux<Response> streamBasic(Request request) {
    testResult =
        ServerTestResult.fromStreamBasic(
            new StreamBasicServerTestResult.Builder().setRequest(request).build());
    return Flux.fromIterable(instruction.getStreamBasic().getStreamPayloads());
  }

  public Flux<Response> streamBasic(Request req, RpcOptions rpcOptions) {
    throw new UnsupportedOperationException();
  }

  public Flux<ResponseWrapper<Response>> streamBasicWrapper(Request req, RpcOptions rpcOptions) {
    throw new UnsupportedOperationException();
  }

  @Override
  public Flux<Response> streamChunkTimeout(Request request) {
    return null;
  }

  @Override
  public Flux<StreamResponse<Response, Response>> streamInitialResponse(Request request) {
    testResult =
        ServerTestResult.fromStreamInitialResponse(
            new StreamInitialResponseServerTestResult.Builder().setRequest(request).build());
    List<StreamResponse<Response, Response>> list = new ArrayList<>();
    list.add(
        StreamResponse.fromFirstResponse(
            instruction.getStreamInitialResponse().getInitialResponse()));
    list.addAll(
        instruction.getStreamInitialResponse().getStreamPayloads().stream()
            .map(StreamResponse::<Response, Response>fromData)
            .collect(Collectors.toList()));

    return Flux.fromIterable(list);
  }

  @Override
  public Flux<Response> streamCreditTimeout(Request request) {
    return null;
  }

  @Override
  public Flux<Response> streamDeclaredException(Request request) {
    testResult =
        ServerTestResult.fromStreamDeclaredException(
            new StreamDeclaredExceptionServerTestResult.Builder().setRequest(request).build());
    return Flux.error(instruction.getStreamDeclaredException().getUserException());
  }

  @Override
  public Flux<Response> streamUndeclaredException(Request request) {
    testResult =
        ServerTestResult.fromStreamUndeclaredException(
            new StreamUndeclaredExceptionServerTestResult.Builder().setRequest(request).build());
    return Flux.error(
        new RuntimeException(instruction.getStreamUndeclaredException().getExceptionMessage()));
  }

  @Override
  public Flux<Response> streamInitialDeclaredException(Request request) {
    testResult =
        ServerTestResult.fromStreamInitialDeclaredException(
            new StreamInitialDeclaredExceptionServerTestResult.Builder()
                .setRequest(request)
                .build());
    UserException ex = instruction.getStreamInitialDeclaredException().getUserException();
    throw new UserException.Builder().setMsg(ex.getMsg()).build();
  }

  @Override
  public Flux<Response> streamInitialUndeclaredException(Request request) {
    testResult =
        ServerTestResult.fromStreamInitialUndeclaredException(
            new StreamInitialUndeclaredExceptionServerTestResult.Builder()
                .setRequest(request)
                .build());
    throw new RuntimeException(
        instruction.getStreamInitialUndeclaredException().getExceptionMessage());
  }

  @Override
  public Flux<Response> streamInitialTimeout(Request request) {
    throw new RuntimeException("Not implemented, needed for client testing only");
  }

  @Override
  public Mono<Response> sinkBasic(Request request, Publisher<Request> publisher) {
    return null;
  }

  @Override
  public Mono<Response> sinkChunkTimeout(Request request, Publisher<Request> publisher) {
    return null;
  }

  @Override
  public Flux<StreamResponse<Response, Response>> sinkInitialResponse(
      Request request, Publisher<Request> publisher) {
    // The return type makes no sense here. Definitely a bug
    return null;
  }

  @Override
  public Mono<Response> sinkDeclaredException(Request request, Publisher<Request> publisher) {
    return null;
  }

  @Override
  public Mono<Response> sinkInitialDeclaredException(
      Request request, Publisher<Request> publisher) {
    return null;
  }

  @Override
  public Mono<Response> sinkUndeclaredException(Request request, Publisher<Request> publisher) {
    return null;
  }

  @Override
  public Mono<Response> sinkServerDeclaredException(Request request, Publisher<Request> publisher) {
    return null;
  }

  @Override
  public BasicInteraction createBasicInteraction() {
    return null;
  }

  @Override
  public void dispose() {}
}
