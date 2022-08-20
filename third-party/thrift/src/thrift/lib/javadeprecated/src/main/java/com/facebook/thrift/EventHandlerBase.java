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

public class EventHandlerBase {
  private int setEventHandlerPos_;
  private ContextStack s_;
  public ArrayList<TProcessorEventHandler> handlers;
  public TProcessorEventHandler eventHandler_;

  public static class ContextStack {
    private ArrayList<Object> ctxs;

    public ContextStack(
        ArrayList<TProcessorEventHandler> handlers,
        String method,
        TConnectionContext connectionContext) {
      ctxs = new ArrayList<Object>();
      for (TProcessorEventHandler handler : handlers) {
        ctxs.add(handler.getContext(method, connectionContext));
      }
    }
  }

  public EventHandlerBase() {
    setEventHandlerPos_ = -1;
    s_ = null;
    handlers = new ArrayList<TProcessorEventHandler>();
  }

  public void addEventHandler(TProcessorEventHandler handler) {
    handlers.add(handler);
  }

  public void clearEventHandlers() {
    handlers.clear();
    setEventHandlerPos_ = -1;
    if (eventHandler_ != null) {
      setEventHandler(eventHandler_);
    }
  }

  public TProcessorEventHandler getEventHandler() {
    return eventHandler_;
  }

  public void setEventHandler(TProcessorEventHandler eventHandler) {
    eventHandler_ = eventHandler;
    if (setEventHandlerPos_ > 0) {
      handlers.remove(setEventHandlerPos_ - 1);
    }
    setEventHandlerPos_ = handlers.size();
    handlers.add(eventHandler);
  }

  /**
   * These functions are only used in the client handler implementation. The server process
   * functions maintain ContextStack on the stack and binds ctx in to the async calls.
   *
   * <p>Clients are not thread safe, so using a member variable is okay. Client send_ and recv_
   * functions contain parameters based off of the function call, and adding a parameter there would
   * change the function signature enough that other thrift users might break.
   *
   * <p>The generated code should be the ONLY user of s_. All other functions should just use the
   * ContextStack parameter.
   */
  public ContextStack getContextStack() {
    return s_;
  }

  // Context only freed by freer, this is only used across function calls.
  public void setContextStack(ContextStack s) {
    s_ = s;
  }

  protected ContextStack getContextStack(String fn_name, TConnectionContext connectionContext) {
    return new ContextStack(handlers, fn_name, connectionContext);
  }

  protected void preWrite(ContextStack s, String fn_name, TBase result) {
    if (s != null) {
      for (int i = 0; i < handlers.size(); i++) {
        try {
          handlers.get(i).preWrite(s.ctxs.get(i), fn_name, result);
        } catch (TException e) {
        }
      }
    }
  }

  protected void postWrite(ContextStack s, String fn_name, TBase result) {
    if (s != null) {
      for (int i = 0; i < handlers.size(); i++) {
        try {
          handlers.get(i).postWrite(s.ctxs.get(i), fn_name, result);
        } catch (TException e) {
        }
      }
    }
  }

  protected void preRead(ContextStack s, String fn_name) {
    if (s != null) {
      for (int i = 0; i < handlers.size(); i++) {
        try {
          handlers.get(i).preRead(s.ctxs.get(i), fn_name);
        } catch (TException e) {
        }
      }
    }
  }

  protected void postRead(ContextStack s, String fn_name, TBase args) {
    if (s != null) {
      for (int i = 0; i < handlers.size(); i++) {
        try {
          handlers.get(i).postRead(s.ctxs.get(i), fn_name, args);
        } catch (TException e) {
        }
      }
    }
  }

  protected void handlerError(ContextStack s, String fn_name, Throwable th) {
    if (s != null) {
      for (int i = 0; i < handlers.size(); i++) {
        try {
          handlers.get(i).handlerError(s.ctxs.get(i), fn_name, th);
        } catch (TException e) {
        }
      }
    }
  }
}
