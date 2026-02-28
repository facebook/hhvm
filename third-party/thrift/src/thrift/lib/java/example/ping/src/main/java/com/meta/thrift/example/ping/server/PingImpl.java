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

package com.meta.thrift.example.ping.server;

import com.facebook.thrift.example.ping.CustomException;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import java.nio.charset.StandardCharsets;
import org.apache.thrift.TException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class PingImpl implements PingService {
  private static final Logger LOG = LoggerFactory.getLogger(PingImpl.class);

  @Override
  public PingResponse ping(PingRequest request) throws TException {
    LOG.info("Get Request: " + request.toString() + " for PingImpl.ping()");
    return new PingResponse("Hello!! " + request.getRequest());
  }

  @Override
  public byte[] pingBinary(PingRequest request) throws TException {
    LOG.info("Get Request: " + request.toString() + " for PingImpl.pingBinary()");
    return "Hello!".getBytes(StandardCharsets.UTF_8);
  }

  @Override
  public PingResponse pingException(PingRequest request) throws CustomException, TException {
    LOG.info("Get Request: " + request.toString() + " for PingImpl.pingException()");
    throw new CustomException("Exception: " + request.getRequest());
  }

  @Override
  public void pingVoid(PingRequest request) throws TException {
    LOG.info("Get Request: " + request.toString() + " for PingImpl.pingVoid()");
  }

  @Override
  public void close() {}
}
