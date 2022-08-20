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
import java.util.ArrayList;
import java.util.List;

/** A class which wraps multiple TProcessorEventHandler objects. */
public class MultipleTProcessorEventHandlers extends TProcessorEventHandler {
  private final List<TProcessorEventHandler> handlers;

  public MultipleTProcessorEventHandlers(List<TProcessorEventHandler> handlers) {
    if (handlers != null) {
      this.handlers = new ArrayList<TProcessorEventHandler>(handlers);
    } else {
      this.handlers = new ArrayList<TProcessorEventHandler>();
    }
  }

  public Object getContext(String fn_name, TConnectionContext context) {
    return new EventHandlerContexts(handlers, fn_name, context);
  }

  /** Invokes each handler's preRead method with its own context. */
  @Override
  public void preRead(Object handler_context, String fn_name) throws TException {
    if (handler_context != null) {
      EventHandlerContexts contexts = (EventHandlerContexts) handler_context;
      for (int i = 0; i < handlers.size(); i++) {
        handlers.get(i).preRead(contexts.ctxs.get(i), fn_name);
      }
    }
  }

  /** Invokes each handler's postRead method with its own context. */
  @Override
  public void postRead(Object handler_context, String fn_name, TBase args) throws TException {
    if (handler_context != null) {
      EventHandlerContexts contexts = (EventHandlerContexts) handler_context;
      for (int i = 0; i < handlers.size(); i++) {
        handlers.get(i).postRead(contexts.ctxs.get(i), fn_name, args);
      }
    }
  }

  /** Invokes each handler's preWrite method with its own context. */
  @Override
  public void preWrite(Object handler_context, String fn_name, TBase result) throws TException {
    if (handler_context != null) {
      EventHandlerContexts contexts = (EventHandlerContexts) handler_context;
      for (int i = 0; i < handlers.size(); i++) {
        handlers.get(i).preWrite(contexts.ctxs.get(i), fn_name, result);
      }
    }
  }

  /** Invokes each handler's preWrite method with its own context. */
  @Override
  public void postWrite(Object handler_context, String fn_name, TBase result) throws TException {
    if (handler_context != null) {
      EventHandlerContexts contexts = (EventHandlerContexts) handler_context;
      for (int i = 0; i < handlers.size(); i++) {
        handlers.get(i).postWrite(contexts.ctxs.get(i), fn_name, result);
      }
    }
  }

  /** Invokes each handler's handlerError method with its own context. */
  @Override
  public void handlerError(Object handler_context, String fn_name, Throwable th) throws TException {
    if (handler_context != null) {
      EventHandlerContexts contexts = (EventHandlerContexts) handler_context;
      for (int i = 0; i < handlers.size(); i++) {
        handlers.get(i).handlerError(contexts.ctxs.get(i), fn_name, th);
      }
    }
  }

  /** Invokes each handler's declaredUserException method with its own context. */
  // NB: for compatibility with Processor code generated in Dec 2015/Jan 2016,
  // this method should have type erasure `(LObject;LString;LThrowable;)V`
  @Override
  public <T extends Throwable & TBase> void declaredUserException(
      Object handler_context, String fn_name, T th) throws TException {
    if (handler_context != null) {
      EventHandlerContexts contexts = (EventHandlerContexts) handler_context;
      for (int i = 0; i < handlers.size(); i++) {
        handlers.get(i).declaredUserException(contexts.ctxs.get(i), fn_name, th);
      }
    }
  }

  /** This class encapsulates the context of each event handler. */
  private static class EventHandlerContexts {
    private final List<Object> ctxs;

    public EventHandlerContexts(
        List<TProcessorEventHandler> handlers,
        String method,
        TConnectionContext connectionContext) {
      ctxs = new ArrayList<Object>();
      for (TProcessorEventHandler handler : handlers) {
        ctxs.add(handler.getContext(method, connectionContext));
      }
    }
  }
}
