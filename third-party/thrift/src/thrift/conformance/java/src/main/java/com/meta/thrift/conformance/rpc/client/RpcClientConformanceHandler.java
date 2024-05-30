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
import org.apache.thrift.conformance.InteractionConstructorClientInstruction;
import org.apache.thrift.conformance.InteractionConstructorClientTestResult;
import org.apache.thrift.conformance.InteractionFactoryFunctionClientInstruction;
import org.apache.thrift.conformance.InteractionPersistsStateClientInstruction;
import org.apache.thrift.conformance.RPCConformanceService;
import org.apache.thrift.conformance.Request;
import org.apache.thrift.conformance.RequestResponseBasicClientInstruction;
import org.apache.thrift.conformance.RequestResponseBasicClientTestResult;
import org.apache.thrift.conformance.RequestResponseDeclaredExceptionClientInstruction;
import org.apache.thrift.conformance.RequestResponseDeclaredExceptionClientTestResult;
import org.apache.thrift.conformance.RequestResponseNoArgVoidResponseClientInstruction;
import org.apache.thrift.conformance.RequestResponseNoArgVoidResponseClientTestResult;
import org.apache.thrift.conformance.RequestResponseTimeoutClientInstruction;
import org.apache.thrift.conformance.RequestResponseTimeoutClientTestResult;
import org.apache.thrift.conformance.RequestResponseUndeclaredExceptionClientInstruction;
import org.apache.thrift.conformance.RequestResponseUndeclaredExceptionClientTestResult;
import org.apache.thrift.conformance.RpcTestCase;
import org.apache.thrift.conformance.SinkBasicClientInstruction;
import org.apache.thrift.conformance.SinkBasicClientTestResult;
import org.apache.thrift.conformance.SinkChunkTimeoutClientInstruction;
import org.apache.thrift.conformance.SinkInitialResponseClientInstruction;
import org.apache.thrift.conformance.StreamBasicClientInstruction;
import org.apache.thrift.conformance.StreamBasicClientTestResult;
import org.apache.thrift.conformance.StreamChunkTimeoutClientInstruction;
import org.apache.thrift.conformance.StreamCreditTimeoutClientInstruction;
import org.apache.thrift.conformance.StreamDeclaredExceptionClientInstruction;
import org.apache.thrift.conformance.StreamDeclaredExceptionClientTestResult;
import org.apache.thrift.conformance.StreamInitialDeclaredExceptionClientInstruction;
import org.apache.thrift.conformance.StreamInitialDeclaredExceptionClientTestResult;
import org.apache.thrift.conformance.StreamInitialResponseClientInstruction;
import org.apache.thrift.conformance.StreamInitialResponseClientTestResult;
import org.apache.thrift.conformance.StreamInitialTimeoutClientInstruction;
import org.apache.thrift.conformance.StreamInitialUndeclaredExceptionClientInstruction;
import org.apache.thrift.conformance.StreamInitialUndeclaredExceptionClientTestResult;
import org.apache.thrift.conformance.StreamUndeclaredExceptionClientInstruction;
import org.apache.thrift.conformance.StreamUndeclaredExceptionClientTestResult;
import org.apache.thrift.conformance.UserException;
import org.apache.thrift.transport.TTransportException;
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
        .thenReturn(ClientTestResult.defaultInstance())
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
        .thenReturn(ClientTestResult.defaultInstance())
        .onErrorResume(
            t ->
                Mono.just(
                    ClientTestResult.fromRequestResponseUndeclaredException(
                        new RequestResponseUndeclaredExceptionClientTestResult.Builder()
                            .setExceptionMessage(t.getMessage())
                            .build())));
  }

  private boolean isTTransportException(Throwable t) {
    return (t instanceof TTransportException
        && ((TTransportException) t).getType() == TTransportException.TIMED_OUT);
  }

  public Mono<ClientTestResult> testRequestResponseTimeout(
      RequestResponseTimeoutClientInstruction instruction) {
    RpcOptions rpcOptions =
        new RpcOptions.Builder().setClientTimeoutMs((int) instruction.getTimeoutMs()).build();

    return client
        .requestResponseTimeout(instruction.getRequest(), rpcOptions)
        .map(r -> ClientTestResult.defaultInstance())
        .onErrorResume(
            t ->
                Mono.just(
                    ClientTestResult.fromRequestResponseTimeout(
                        new RequestResponseTimeoutClientTestResult.Builder()
                            .setTimeoutException(isTTransportException(t))
                            .build())));
  }

  public Mono<ClientTestResult> testStreamBasic(StreamBasicClientInstruction instruction) {
    return client
        .streamBasic(instruction.getRequest())
        .limitRate(10, 3)
        .collectList()
        .map(
            r ->
                ClientTestResult.fromStreamBasic(
                    new StreamBasicClientTestResult.Builder().setStreamPayloads(r).build()));
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

  public Mono<ClientTestResult> testStreamDeclaredException(
      StreamDeclaredExceptionClientInstruction instruction) {
    return client
        .streamDeclaredException(instruction.getRequest())
        .last()
        .map(r -> ClientTestResult.defaultInstance())
        .onErrorResume(
            t ->
                Mono.just(
                    ClientTestResult.fromStreamDeclaredException(
                        new StreamDeclaredExceptionClientTestResult.Builder()
                            .setUserException((UserException) t)
                            .build())));
  }

  public Mono<ClientTestResult> testStreamUndeclaredException(
      StreamUndeclaredExceptionClientInstruction instruction) {
    return client
        .streamUndeclaredException(instruction.getRequest())
        .last()
        .map(r -> ClientTestResult.defaultInstance())
        .onErrorResume(
            t ->
                Mono.just(
                    ClientTestResult.fromStreamUndeclaredException(
                        new StreamUndeclaredExceptionClientTestResult.Builder()
                            .setExceptionMessage(t.getMessage())
                            .build())));
  }

  public Mono<ClientTestResult> testStreamInitialDeclaredException(
      StreamInitialDeclaredExceptionClientInstruction instruction) {
    return client
        .streamInitialDeclaredException(instruction.getRequest())
        .last()
        .map(r -> ClientTestResult.defaultInstance())
        .onErrorResume(
            t ->
                Mono.just(
                    ClientTestResult.fromStreamInitialDeclaredException(
                        new StreamInitialDeclaredExceptionClientTestResult.Builder()
                            .setUserException((UserException) t)
                            .build())));
  }

  public Mono<ClientTestResult> testStreamInitialUndeclaredException(
      StreamInitialUndeclaredExceptionClientInstruction instruction) {
    return client
        .streamInitialUndeclaredException(instruction.getRequest())
        .last()
        .map(r -> ClientTestResult.defaultInstance())
        .onErrorResume(
            t ->
                Mono.just(
                    ClientTestResult.fromStreamInitialUndeclaredException(
                        new StreamInitialUndeclaredExceptionClientTestResult.Builder()
                            .setExceptionMessage(t.getMessage())
                            .build())));
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

  public Mono<ClientTestResult> testStreamChunkTimeout(
      StreamChunkTimeoutClientInstruction instruction) {
    throw new RuntimeException("Not supported");
  }

  public Mono<ClientTestResult> testStreamCreditTimeout(
      StreamCreditTimeoutClientInstruction instruction) {
    throw new RuntimeException("Not supported");
  }

  public Mono<ClientTestResult> testStreamInitialTimeout(
      StreamInitialTimeoutClientInstruction instruction) {
    throw new RuntimeException("Not supported");
  }

  public Mono<ClientTestResult> testSinkChunkTimeout(
      SinkChunkTimeoutClientInstruction instruction) {
    throw new RuntimeException("Not supported");
  }

  public Mono<ClientTestResult> testSinkInitialResponse(
      SinkInitialResponseClientInstruction instruction) {
    throw new RuntimeException("Not supported");
  }

  public Mono<ClientTestResult> testInteractionConstructor(
      InteractionConstructorClientInstruction instruction) {
    return client
        .createBasicInteraction()
        .init()
        .thenReturn(
            ClientTestResult.fromInteractionConstructor(
                new InteractionConstructorClientTestResult.Builder().build()));
  }

  public Mono<ClientTestResult> testInteractionFactoryFunction(
      InteractionFactoryFunctionClientInstruction instruction) {
    //    return client
    //            .basicInteractionFactoryFunction(instruction.getInitialSum())
    //            .thenReturn(ClientTestResult.fromInteractionFactoryFunction(new
    // InteractionFactoryFunctionClientTestResult.Builder().build()));
    throw new RuntimeException("Not supported");
  }

  public Mono<ClientTestResult> testInteractionPersistsState(
      InteractionPersistsStateClientInstruction instruction) {
    RPCConformanceService.Reactive.BasicInteraction interaction = client.createBasicInteraction();

    //    return interaction
    //            .init()
    //            .thenReturn(instruction.getValuesToAdd())
    //            .flatMapIterable(i -> i)
    //            .map(i  -> interaction.add(i))
    //            .flatMap(i->i)
    //            .collectList()
    //            .map(list->ClientTestResult.fromInteractionPersistsState(new
    // InteractionPersistsStateClientTestResult.Builder().setResponses(list.stream().collect(Collectors.toList())).build()));
    throw new RuntimeException("Not supported");
  }
}
