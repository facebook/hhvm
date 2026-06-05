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

package com.facebook.swift.service;

import com.facebook.nifty.core.RequestContext;
import java.util.ArrayList;
import java.util.List;
import org.apache.thrift.TException;

/**
 * Per-request fan-out helper that invokes each {@link ThriftEventHandler} at the matching lifecycle
 * point with the per-handler {@code context} returned by {@link ThriftEventHandler#getContext}.
 * First handler that throws short-circuits the rest.
 */
public class ContextChain {
  private final List<ThriftEventHandler> handlers;
  private final String methodName;
  private final List<Object> contexts;

  /**
   * Calls {@code getContext} on every handler once and stores the result. If a later handler's
   * {@code getContext} throws, unwinds the already-succeeded prefix by calling {@code done()} in
   * reverse, then rethrows -- preserves the per-handler invariant that a successful {@code
   * getContext} is always paired with a {@code done()}.
   */
  public ContextChain(
      List<ThriftEventHandler> handlers, String methodName, RequestContext requestContext) {
    this.handlers = handlers;
    this.methodName = methodName;
    this.contexts = new ArrayList<>();
    try {
      for (ThriftEventHandler h : this.handlers) {
        this.contexts.add(h.getContext(methodName, requestContext));
      }
    } catch (Throwable t) {
      for (int i = contexts.size() - 1; i >= 0; i--) {
        try {
          this.handlers.get(i).done(contexts.get(i), methodName);
        } catch (Throwable suppressed) {
          t.addSuppressed(suppressed);
        }
      }
      throw t;
    }
  }

  /**
   * Admission-control hook: invoked on the caller thread BEFORE off-loop queueing so a shed never
   * pays queue time, parse CPU, or pins the request buffer.
   */
  public void preprocess() throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).preprocess(contexts.get(i), methodName);
    }
  }

  public void preRead() throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).preRead(contexts.get(i), methodName);
    }
  }

  public void postRead(Object[] args) throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).postRead(contexts.get(i), methodName, args);
    }
  }

  public void postRead(List args) throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).postRead(contexts.get(i), methodName, args);
    }
  }

  public void preWrite(Object result) throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).preWrite(contexts.get(i), methodName, result);
    }
  }

  public void preWriteException(Throwable t) throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).preWriteException(contexts.get(i), methodName, t);
    }
  }

  public void postWrite(Object result) throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).postWrite(contexts.get(i), methodName, result);
    }
  }

  public void postWriteException(Throwable t) throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).postWriteException(contexts.get(i), methodName, t);
    }
  }

  public void declaredUserException(Throwable t) throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).declaredUserException(contexts.get(i), methodName, t);
    }
  }

  public void undeclaredUserException(Throwable t) throws TException {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).undeclaredUserException(contexts.get(i), methodName, t);
    }
  }

  public void done() {
    for (int i = 0; i < handlers.size(); i++) {
      handlers.get(i).done(contexts.get(i), methodName);
    }
  }
}
