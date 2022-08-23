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
import java.util.HashMap;
import java.util.Map;
import org.apache.thrift.TException;
import org.apache.thrift.conformance.ClientTestResult;
import org.apache.thrift.conformance.RPCConformanceService;
import org.apache.thrift.conformance.Request;
import org.apache.thrift.conformance.RequestResponseBasicServerTestResult;
import org.apache.thrift.conformance.Response;
import org.apache.thrift.conformance.RpcTestCase;
import org.apache.thrift.conformance.ServerInstruction;
import org.apache.thrift.conformance.ServerTestResult;
import org.apache.thrift.conformance.UserException;

public class RpcServerConformanceHandler implements RPCConformanceService {
  private Map<Request, Response> requestResponseMap = new HashMap<>();

  @Override
  public void close() {}

  private ServerInstruction instruction;
  private ServerTestResult testResult;

  @Override
  public void sendTestCase(RpcTestCase rpcTestCase) throws TException {
    instruction = rpcTestCase.getServerInstruction();
  }

  @Override
  public void sendTestCase(RpcTestCase testCase, RpcOptions rpcOptions) throws TException {}

  @Override
  public ResponseWrapper<Void> sendTestCaseWrapper(RpcTestCase testCase, RpcOptions rpcOptions)
      throws TException {
    return null;
  }

  @Override
  public ServerTestResult getTestResult() throws TException {
    return testResult;
  }

  @Override
  public ServerTestResult getTestResult(RpcOptions rpcOptions) throws TException {
    return null;
  }

  @Override
  public ResponseWrapper<ServerTestResult> getTestResultWrapper(RpcOptions rpcOptions)
      throws TException {
    return null;
  }

  @Override
  public Response requestResponseBasic(Request request) throws TException {
    testResult =
        ServerTestResult.fromRequestResponseBasic(
            new RequestResponseBasicServerTestResult.Builder().setRequest(request).build());
    return instruction.getRequestResponseBasic().getResponse();
  }

  @Override
  public RpcTestCase getTestCase() throws TException {
    return null;
  }

  @Override
  public void sendTestResult(ClientTestResult clientTestResult) throws TException {}

  @Override
  public void requestResponseDeclaredException(Request request) throws UserException, TException {}

  @Override
  public void requestResponseUndeclaredException(Request request) throws TException {}

  @Override
  public void requestResponseNoArgVoidResponse() throws TException {}

  @Override
  public RpcTestCase getTestCase(RpcOptions rpcOptions) throws TException {
    return null;
  }

  @Override
  public ResponseWrapper<RpcTestCase> getTestCaseWrapper(RpcOptions rpcOptions) throws TException {
    return null;
  }

  @Override
  public void sendTestResult(ClientTestResult result, RpcOptions rpcOptions) throws TException {}

  @Override
  public ResponseWrapper<Void> sendTestResultWrapper(ClientTestResult result, RpcOptions rpcOptions)
      throws TException {
    return null;
  }

  @Override
  public Response requestResponseBasic(Request req, RpcOptions rpcOptions) throws TException {
    return null;
  }

  @Override
  public ResponseWrapper<Response> requestResponseBasicWrapper(Request req, RpcOptions rpcOptions)
      throws TException {
    return null;
  }

  @Override
  public void requestResponseDeclaredException(Request req, RpcOptions rpcOptions)
      throws UserException, TException {}

  @Override
  public ResponseWrapper<Void> requestResponseDeclaredExceptionWrapper(
      Request req, RpcOptions rpcOptions) throws UserException, TException {
    return null;
  }

  @Override
  public void requestResponseUndeclaredException(Request req, RpcOptions rpcOptions)
      throws TException {}

  @Override
  public ResponseWrapper<Void> requestResponseUndeclaredExceptionWrapper(
      Request req, RpcOptions rpcOptions) throws TException {
    return null;
  }

  @Override
  public void requestResponseNoArgVoidResponse(RpcOptions rpcOptions) throws TException {}

  @Override
  public ResponseWrapper<Void> requestResponseNoArgVoidResponseWrapper(RpcOptions rpcOptions)
      throws TException {
    return null;
  }

  @Override
  public Response requestResponseTimeout(Request request) throws TException {
    return null;
  }

  @Override
  public Response requestResponseTimeout(Request req, RpcOptions rpcOptions) throws TException {
    return null;
  }

  @Override
  public ResponseWrapper<Response> requestResponseTimeoutWrapper(Request req, RpcOptions rpcOptions)
      throws TException {
    return null;
  }
}
