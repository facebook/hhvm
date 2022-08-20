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

import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.server.TConnectionContext;
import com.facebook.thrift.transport.THeaderTransport;
import com.facebook.thrift.transport.TTransport;

public class UserExceptionHandler extends TProcessorEventHandler {
  public Object getContext(String fn_name, TConnectionContext connectionContext) {

    TProtocol protocol = connectionContext.getOutputProtocol();
    TTransport transport = protocol.getTransport();
    return (transport instanceof THeaderTransport) ? transport : null;
  }

  @Override
  public <T extends Throwable & TBase> void declaredUserException(
      Object handler_context, String fn_name, T th) throws TException {

    if (handler_context != null) {
      THeaderTransport headerTransport = (THeaderTransport) handler_context;
      headerTransport.setHeader("uex", th.getClass().getSimpleName());
      // On thrift-generated exceptions, getMessage doesn't include any
      // declared fields (and will be unset unless somebody explicitly called
      // `th.setMessage()`) and toString includes declared fields but not the
      // message. If whoever made the exception did set a message, use that.
      // Otherwise, fall back to toString. It would be nice to skip toString if
      // there are no fields, but that would be much harder.
      String message = th.getMessage();
      if (message != null && !message.isEmpty()) {
        headerTransport.setHeader("uexw", message);
      } else {
        String str = // don't inline; see JDK-8056984
            th.toString(1, /* prettyPrint */ false);
        headerTransport.setHeader("uexw", str);
      }
    }
  }

  @Override
  public void handlerError(Object handler_context, String fn_name, Throwable th) throws TException {

    if (handler_context != null) {
      THeaderTransport headerTransport = (THeaderTransport) handler_context;
      headerTransport.setHeader("uex", th.getClass().getSimpleName());
      headerTransport.setHeader("uexw", th.toString());
    }
  }
}
