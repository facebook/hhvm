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

import com.facebook.nifty.core.RequestContext;

public class ThriftServerStatsContext {

  final RequestContext requestContext;
  long readStartTime;
  long writeStartTime;
  long processStartTime;

  ThriftServerStatsContext(RequestContext requestContext) {
    this.requestContext = requestContext;
  }

  public long getReadStartTime() {
    return this.readStartTime;
  }

  public long getWriteStartTime() {
    return this.writeStartTime;
  }

  public long getProcessStartTime() {
    return this.processStartTime;
  }

  public RequestContext getRequestContext() {
    return this.requestContext;
  }

  public void setReadStartTime(long readStartTime) {
    this.readStartTime = readStartTime;
  }

  public void setWriteStartTime(long writeStartTime) {
    this.writeStartTime = writeStartTime;
  }

  public void setProcessStartTime(long processStartTime) {
    this.processStartTime = processStartTime;
  }
}
