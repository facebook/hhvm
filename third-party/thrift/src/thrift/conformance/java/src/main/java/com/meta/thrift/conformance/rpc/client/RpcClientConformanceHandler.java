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
import java.lang.reflect.Method;
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

  private Mono<ClientTestResult> execute(ClientInstruction instruction) {
    try {
      String name =
          instruction.getThriftName().substring(0, 1).toUpperCase()
              + instruction.getThriftName().substring(1);
      Method getter = ClientInstruction.class.getMethod("get" + name, null);
      Object o = getter.invoke(instruction);
      Method test = this.getClass().getMethod("test" + name, o.getClass());
      return (Mono<ClientTestResult>) test.invoke(this, o);
    } catch (Throwable t) {
      throw new RuntimeException(t);
    }
  }

  /**
   * All tests are executed using reflection to enable loosely coupling between test definitions and
   * the implementation. Any additional tests introduced in the conformance test framework should
   * not cause any build time failure. Additional tests should be added to the nonconforming.txt
   * file for exclusion.
   */
  public void executeTests() {
    Mono<Void> test =
        client
            .getTestCase()
            .map(RpcTestCase::getClientInstruction)
            .flatMap(this::execute)
            .onErrorStop()
            .flatMap(client::sendTestResult);

    StepVerifier.create(test).verifyComplete();
  }

  public Mono<ClientTestResult> testRequestResponseBasic(
      RequestResponseBasicClientInstruction instruction) {
    return client
        .requestResponseBasic(instruction.getRequest())
        .map(
            r ->
                ClientTestResult.fromRequestResponseBasic(
                    new RequestResponseBasicClientTestResult.Builder().setResponse(r).build()));
  }

  public Mono<ClientTestResult> testRequestResponseDeclaredException(
      RequestResponseDeclaredExceptionClientInstruction instruction) {
    return client
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

  public Mono<ClientTestResult> testRequestResponseNoArgVoidResponse(
      RequestResponseNoArgVoidResponseClientInstruction instruction) {
    return client
        .requestResponseNoArgVoidResponse()
        .thenReturn(
            ClientTestResult.fromRequestResponseNoArgVoidResponse(
                new RequestResponseNoArgVoidResponseClientTestResult.Builder().build()));
  }

  public Mono<ClientTestResult> testRequestResponseUndeclaredException(
      RequestResponseUndeclaredExceptionClientInstruction instruction) {
    return client
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

  public Mono<ClientTestResult> testStreamBasic(StreamBasicClientInstruction instruction) {
    return client
        .streamBasic(instruction.getRequest())
        .collectList()
        .map(
            r ->
                ClientTestResult.fromStreamBasic(
                    new StreamBasicClientTestResult.Builder().setStreamPayloads(r).build()));
  }

  public Mono<ClientTestResult> testStreamChunkTimeout(
      StreamChunkTimeoutClientInstruction instruction) {
    RpcOptions rpcOptions =
        new RpcOptions.Builder().setClientTimeoutMs((int) instruction.getChunkTimeoutMs()).build();

    return client
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

  public Mono<ClientTestResult> testSinkBasic(SinkBasicClientInstruction instruction) {
    Publisher<Request> publisher = Flux.fromIterable(instruction.getSinkPayloads());

    return client
        .sinkBasic(instruction.getRequest(), publisher)
        .map(
            r ->
                ClientTestResult.fromSinkBasic(
                    new SinkBasicClientTestResult.Builder().setFinalResponse(r).build()));
  }

  public Mono<ClientTestResult> testStreamInitialResponse(
      StreamInitialResponseClientInstruction instruction) {
    return client
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

  public Mono<ClientTestResult> testRequestResponseTimeout(
      RequestResponseTimeoutClientInstruction instruction) {
    return null;
  }

  public Mono<ClientTestResult> testSinkChunkTimeout(
      SinkChunkTimeoutClientInstruction instruction) {
    return null;
  }

  public Mono<ClientTestResult> testStreamCreditTimeout(
      StreamCreditTimeoutClientInstruction instruction) {
    return null;
  }
}
