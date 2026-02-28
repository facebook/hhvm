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

package com.facebook.thrift;

import com.facebook.thrift.server.TConnectionContext;

/** An event handler for a TProcessor. */
public class TProcessorEventHandler {
  public Object getContext(String fn_name, TConnectionContext context) {
    return null;
  }

  /**
   * Called before the thrift handler method's arguments are read.
   *
   * @param handler_context object returned by {@link getContext} call
   * @param fn_name name of the thrift method being invoked
   */
  public void preRead(Object handler_context, String fn_name) throws TException {}

  /**
   * Called after the thrift handler method's arguments are read.
   *
   * @param handler_context object returned by {@link getContext} call
   * @param fn_name name of the thrift method being invoked
   * @param args the thrift structure holding the method's arguments
   */
  public void postRead(Object handler_context, String fn_name, TBase args) throws TException {}

  /**
   * Called before the thrift handler method's results are written.
   *
   * @param handler_context object returned by {@link getContext} call
   * @param fn_name name of the thrift method being invoked
   * @param result instance of TBase holding the result of the call, or null if the handler threw an
   *     unexpected exception
   */
  public void preWrite(Object handler_context, String fn_name, TBase result) throws TException {}

  /**
   * Called after the thrift handler method's results are written.
   *
   * @param handler_context object returned by {@link getContext} call
   * @param fn_name name of the thrift method being invoked
   * @param result instance of TBase holding the result of the call, or null if the handler threw an
   *     unexpected exception
   */
  public void postWrite(Object handler_context, String fn_name, TBase result) throws TException {}

  /**
   * Called before the thrift handler method's results are written iff there was an error.
   *
   * @param handler_context object returned by {@link getContext} call
   * @param fn_name name of the thrift method being invoked
   * @param th the unexpected exception thrown by the thrift handler
   */
  public void handlerError(Object handler_context, String fn_name, Throwable th)
      throws TException {}

  /**
   * Called before the thrift handler method's results are written iff an exception is thrown that
   * is declared in IDL and declared as throwable by the currently processing server method.
   *
   * @param handler_context object returned by {@link getContext} call
   * @param fn_name name of the thrift method being invoked
   * @param th the exception thrown by the thrift handler
   */
  // NB: for compatibility with Processor code generated in Dec 2015/Jan 2016,
  // this method should have type erasure `(LObject;LString;LThrowable;)V`
  public <T extends Throwable & TBase> void declaredUserException(
      Object handler_context, String fn_name, T th) throws TException {}
}
