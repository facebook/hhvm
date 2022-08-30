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

package com.facebook.swift.service.stats;

import static java.lang.System.currentTimeMillis;

import com.facebook.nifty.core.RequestContext;
import com.facebook.swift.service.ThriftEventHandler;
import com.google.common.base.Preconditions;
import io.netty.util.internal.OutOfDirectMemoryError;

/**
 * Handler which needs to be injected to Thrift server processor for capturing ServerStats. An
 * instance of ThriftServerStats is needed to initialize the handler. It uses preRead(), postRead()
 * preWrite(), postWrite() and done() methods from parent class to manage statistics.
 */
public class ThriftServerStatsHandler extends ThriftEventHandler {

  private final ServerStats thriftServerStats;

  public ThriftServerStatsHandler(ServerStats thriftServerStats) throws NullPointerException {
    Preconditions.checkNotNull(thriftServerStats);
    this.thriftServerStats = thriftServerStats;
  }

  @Override
  public Object getContext(String methodName, RequestContext requestContext) {
    ThriftServerStatsContext context = new ThriftServerStatsContext(requestContext);
    context.setProcessStartTime(currentTimeMillis());
    return context;
  }

  @Override
  public void preRead(Object context, String methodName) {
    if (context instanceof ThriftServerStatsContext) {
      ThriftServerStatsContext ctx = (ThriftServerStatsContext) context;
      ctx.setReadStartTime(currentTimeMillis());
    }
  }

  @Override
  public void postRead(Object context, String methodName, Object[] args) {
    if (context instanceof ThriftServerStatsContext) {
      ThriftServerStatsContext ctx = (ThriftServerStatsContext) context;
      thriftServerStats.requestReceived(
          (currentTimeMillis() - ctx.getReadStartTime()) * 1000, methodName);
    }
  }

  @Override
  public void preWrite(Object context, String methodName, Object result) {
    if (context instanceof ThriftServerStatsContext) {
      ThriftServerStatsContext ctx = (ThriftServerStatsContext) context;
      ctx.setWriteStartTime(currentTimeMillis());
    }
  }

  @Override
  public void preWriteException(Object context, String methodName, Throwable t) {
    if (t instanceof OutOfDirectMemoryError) {
      thriftServerStats.markDirectOomError();
    }
    thriftServerStats.error(methodName);
  }

  @Override
  public void postWrite(Object context, String methodName, Object result) {
    if (context instanceof ThriftServerStatsContext) {
      ThriftServerStatsContext ctx = (ThriftServerStatsContext) context;
      thriftServerStats.publishWriteTime((currentTimeMillis() - ctx.getWriteStartTime()) * 1000);
    }
  }

  @Override
  public void done(Object context, String methodName) {
    if (context instanceof ThriftServerStatsContext) {
      ThriftServerStatsContext ctx = (ThriftServerStatsContext) context;
      thriftServerStats.replySent(
          (currentTimeMillis() - ctx.getProcessStartTime()) * 1000, methodName);
    }
  }
}
