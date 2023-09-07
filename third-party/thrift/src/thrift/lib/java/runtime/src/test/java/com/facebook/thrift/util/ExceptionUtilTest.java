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

package com.facebook.thrift.util;

import static org.junit.Assert.assertEquals;

import com.facebook.thrift.payload.ClientResponsePayload;
import java.util.Collections;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TBaseException;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TProtocolException;
import org.apache.thrift.transport.TTransportException;
import org.junit.Test;
import reactor.core.Exceptions;

public class ExceptionUtilTest {

  private class MyException extends TBaseException {
    public MyException(String msg) {
      super(msg);
    }
  }

  @Test
  public void testTApplicationException() {
    TApplicationException ex = new TApplicationException("exc");
    TException result = ExceptionUtil.wrap(ex);
    assertEquals(ex, result);
  }

  @Test
  public void testTException() {
    TException ex = new TException("exc");
    TException result = ExceptionUtil.wrap(ex);
    assertEquals(ex, result);
  }

  @Test
  public void testCustomException() {
    NullPointerException npe = new NullPointerException("npe");
    TException result = ExceptionUtil.wrap(npe);
    assertEquals(npe, result.getCause());

    MyException myExp = new MyException("my exception");
    result = ExceptionUtil.wrap(myExp);
    assertEquals(myExp, result.getCause());
  }

  @Test
  public void testReactorException() {
    MyException myExp = new MyException("my exception");
    TException result = ExceptionUtil.wrap(Exceptions.propagate(myExp));
    assertEquals(myExp, result.getCause());
  }

  @Test
  public void testPropagateNoTransformation() {
    MyException myExp = new MyException("my exception");

    ResponseRpcMetadata metadata =
        new ResponseRpcMetadata(
            Collections.singletonMap("ex", "x"), 0L, 0, null, null, null, 0, null, null, null);

    ClientResponsePayload responsePayload =
        ClientResponsePayload.createException(myExp, metadata, null, false);
    RuntimeException ex = ExceptionUtil.propagate(responsePayload);
    assertEquals(myExp, ex);
    assertEquals("x", ((MyException) ex).headers().get("ex"));
  }

  @Test
  public void testPropagateTransportException() {
    MyException myExp = new MyException("my exception");

    ResponseRpcMetadata metadata =
        new ResponseRpcMetadata(
            Collections.singletonMap("ex", "3"), 0L, 0, null, null, null, 0, null, null, null);

    ClientResponsePayload responsePayload =
        ClientResponsePayload.createException(myExp, metadata, null, false);
    RuntimeException ex = ExceptionUtil.propagate(responsePayload);
    assertEquals(TTransportException.class, ex.getClass());
    assertEquals("my exception", ex.getMessage());
    assertEquals("3", ((TTransportException) ex).headers().get("ex"));
  }

  @Test
  public void testPropagateTransportExceptionNullMsg() {
    MyException myExp = new MyException(null);

    ResponseRpcMetadata metadata =
        new ResponseRpcMetadata(
            Collections.singletonMap("ex", "3"), 0L, 0, null, null, null, 0, null, null, null);

    ClientResponsePayload responsePayload =
        ClientResponsePayload.createException(myExp, metadata, null, false);
    RuntimeException ex = ExceptionUtil.propagate(responsePayload);
    assertEquals(TTransportException.class, ex.getClass());
    assertEquals("Proxy hit a transport exception", ex.getMessage());
  }

  @Test
  public void testPropagateProtocolException() {
    MyException myExp = new MyException("my exception");

    ResponseRpcMetadata metadata =
        new ResponseRpcMetadata(
            Collections.singletonMap("ex", "4"), 0L, 0, null, null, null, 0, null, null, null);

    ClientResponsePayload responsePayload =
        ClientResponsePayload.createException(myExp, metadata, null, false);
    RuntimeException ex = ExceptionUtil.propagate(responsePayload);
    assertEquals(TProtocolException.class, ex.getClass());
    assertEquals("Proxy hit a protocol exception", ex.getMessage());
  }

  @Test
  public void testPropagateNoHeader() {
    MyException myExp = new MyException("my exception");

    ResponseRpcMetadata metadata =
        new ResponseRpcMetadata(
            Collections.emptyMap(), 0L, 0, null, null, null, 0, null, null, null);

    ClientResponsePayload responsePayload =
        ClientResponsePayload.createException(myExp, metadata, null, false);
    RuntimeException ex = ExceptionUtil.propagate(responsePayload);
    assertEquals(myExp, ex);
  }

  @Test
  public void testPropagateHeader() {
    TException myExp = new TException("exc");

    ResponseRpcMetadata metadata =
        new ResponseRpcMetadata(
            Collections.singletonMap("ex", "5"), 0L, 0, null, null, null, 0, null, null, null);

    ClientResponsePayload responsePayload =
        ClientResponsePayload.createException(myExp, metadata, null, false);
    RuntimeException ex = ExceptionUtil.propagate(responsePayload);
    assertEquals(myExp, ex);
    assertEquals("5", ((TException) ex).headers().get("ex"));
  }
}
