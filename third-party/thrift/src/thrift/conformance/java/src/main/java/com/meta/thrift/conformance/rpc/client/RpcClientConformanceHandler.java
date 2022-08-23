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

package com.meta.thrift.conformance.rpc.client;

import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.RpcOptions;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.rsocket.client.RSocketRpcClientFactory;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.stream.Collectors;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.conformance.ClientInstruction;
import org.apache.thrift.conformance.ClientTestResult;
import org.apache.thrift.conformance.RPCConformanceService;
import org.apache.thrift.conformance.Request;
import org.apache.thrift.conformance.RequestResponseBasicClientInstruction;
import org.apache.thrift.conformance.RequestResponseBasicClientTestResult;
import org.apache.thrift.conformance.RequestResponseDeclaredExceptionClientInstruction;
import org.apache.thrift.conformance.RequestResponseDeclaredExceptionClientTestResult;
import org.apache.thrift.conformance.RequestResponseNoArgVoidResponseClientInstruction;
import org.apache.thrift.conformance.RequestResponseNoArgVoidResponseClientTestResult;
import org.apache.thrift.conformance.RequestResponseTimeoutClientInstruction;
import org.apache.thrift.conformance.RequestResponseUndeclaredExceptionClientInstruction;
import org.apache.thrift.conformance.RequestResponseUndeclaredExceptionClientTestResult;
import org.apache.thrift.conformance.RpcTestCase;
import org.apache.thrift.conformance.SinkBasicClientInstruction;
import org.apache.thrift.conformance.SinkBasicClientTestResult;
import org.apache.thrift.conformance.SinkChunkTimeoutClientInstruction;
import org.apache.thrift.conformance.StreamBasicClientInstruction;
import org.apache.thrift.conformance.StreamBasicClientTestResult;
import org.apache.thrift.conformance.StreamChunkTimeoutClientInstruction;
import org.apache.thrift.conformance.StreamChunkTimeoutClientTestResult;
import org.apache.thrift.conformance.StreamCreditTimeoutClientInstruction;
import org.apache.thrift.conformance.StreamInitialResponseClientInstruction;
import org.apache.thrift.conformance.StreamInitialResponseClientTestResult;
import org.apache.thrift.conformance.UserException;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.test.StepVerifier;

public class RpcClientConformanceHandler {

  private RPCConformanceService.Reactive client;

  public RpcClientConformanceHandler(int port) {
    initClient(port);
  }

  private void initClient(int port) {
    final RpcClientFactory clientFactory =
        new RSocketRpcClientFactory(new ThriftClientConfig().setDisableSSL(true));
    SocketAddress address = InetSocketAddress.createUnresolved("::1", port);

    this.client =
        RPCConformanceService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.BINARY)
            .build(clientFactory, address);
  }

  public void executeTests() {
    Mono<Void> test =
        client
            .getTestCase()
            .map(RpcTestCase::getClientInstruction)
            .flatMap(
                instruction -> {
                  Test v = new Test();
                  instruction.accept(v);
                  return v.getResult();
                })
            .flatMap(client::sendTestResult);

    StepVerifier.create(test).verifyComplete();
  }

  private class Test implements ClientInstruction.Visitor {

    private Mono<ClientTestResult> result;

    public Mono<ClientTestResult> getResult() {
      return this.result;
    }

    @Override
    public void visitRequestResponseBasic(RequestResponseBasicClientInstruction instruction) {
      result =
          client
              .requestResponseBasic(instruction.getRequest())
              .map(
                  r ->
                      ClientTestResult.fromRequestResponseBasic(
                          new RequestResponseBasicClientTestResult.Builder()
                              .setResponse(r)
                              .build()));
    }

    @Override
    public void visitRequestResponseDeclaredException(
        RequestResponseDeclaredExceptionClientInstruction instruction) {
      result =
          client
              .requestResponseDeclaredException(instruction.getRequest())
              .cast(ClientTestResult.class)
              .onErrorResume(
                  t ->
                      Mono.just(
                          ClientTestResult.fromRequestResponseDeclaredException(
                              new RequestResponseDeclaredExceptionClientTestResult.Builder()
                                  .setUserException((UserException) t)
                                  .build())));
    }

    @Override
    public void visitRequestResponseNoArgVoidResponse(
        RequestResponseNoArgVoidResponseClientInstruction instruction) {
      result =
          client
              .requestResponseNoArgVoidResponse()
              .thenReturn(
                  ClientTestResult.fromRequestResponseNoArgVoidResponse(
                      new RequestResponseNoArgVoidResponseClientTestResult.Builder().build()));
    }

    @Override
    public void visitRequestResponseUndeclaredException(
        RequestResponseUndeclaredExceptionClientInstruction instruction) {
      result =
          client
              .requestResponseUndeclaredException(instruction.getRequest())
              .cast(ClientTestResult.class)
              .onErrorResume(
                  t ->
                      Mono.just(
                          ClientTestResult.fromRequestResponseUndeclaredException(
                              new RequestResponseUndeclaredExceptionClientTestResult.Builder()
                                  .setExceptionMessage(t.getMessage())
                                  .build())));
    }

    @Override
    public void visitRequestResponseTimeout(
        RequestResponseTimeoutClientInstruction requestResponseTimeoutClientInstruction) {}

    @Override
    public void visitStreamBasic(StreamBasicClientInstruction instruction) {
      result =
          client
              .streamBasic(instruction.getRequest())
              .collectList()
              .map(
                  r ->
                      ClientTestResult.fromStreamBasic(
                          new StreamBasicClientTestResult.Builder().setStreamPayloads(r).build()));
    }

    @Override
    public void visitStreamChunkTimeout(StreamChunkTimeoutClientInstruction instruction) {
      RpcOptions rpcOptions =
          new RpcOptions.Builder()
              .setClientTimeoutMs((int) instruction.getChunkTimeoutMs())
              .build();

      result =
          client
              .streamChunkTimeout(instruction.getRequest(), rpcOptions)
              .collectList()
              .map(
                  r ->
                      ClientTestResult.fromStreamChunkTimeout(
                          new StreamChunkTimeoutClientTestResult.Builder()
                              .setStreamPayloads(r)
                              .setChunkTimeoutException(true)
                              .build()));
    }

    @Override
    public void visitSinkBasic(SinkBasicClientInstruction instruction) {
      Publisher<Request> publisher = Flux.fromIterable(instruction.getSinkPayloads());

      result =
          client
              .sinkBasic(instruction.getRequest(), publisher)
              .map(
                  r ->
                      ClientTestResult.fromSinkBasic(
                          new SinkBasicClientTestResult.Builder().setFinalResponse(r).build()));
    }

    @Override
    public void visitSinkChunkTimeout(
        SinkChunkTimeoutClientInstruction sinkChunkTimeoutClientInstruction) {}

    @Override
    public void visitStreamInitialResponse(StreamInitialResponseClientInstruction instruction) {
      result =
          client
              .streamInitialResponse(instruction.getRequest())
              .collectList()
              .map(
                  r ->
                      ClientTestResult.fromStreamInitialResponse(
                          new StreamInitialResponseClientTestResult.Builder()
                              .setInitialResponse(r.get(0).getFirstResponse())
                              .setStreamPayloads(
                                  r.stream()
                                      .filter(s -> s.isSetData())
                                      .map(s -> s.getData())
                                      .collect(Collectors.toList()))
                              .build()));
    }

    @Override
    public void visitStreamCreditTimeout(
        StreamCreditTimeoutClientInstruction streamCreditTimeoutClientInstruction) {}
  }
}
