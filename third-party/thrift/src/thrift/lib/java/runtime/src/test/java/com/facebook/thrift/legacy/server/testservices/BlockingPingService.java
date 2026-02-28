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

package com.facebook.thrift.legacy.server.testservices;

import com.facebook.thrift.example.ping.CustomException;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.atomic.AtomicInteger;
import org.apache.thrift.TException;

public class BlockingPingService implements PingService {
  private final AtomicInteger counter = new AtomicInteger();

  @Override
  public void close() {}

  @Override
  public PingResponse ping(PingRequest pingRequest) throws TException {
    return new PingResponse.Builder()
        .setResponse(pingRequest.getRequest() + "_pong_" + counter.getAndIncrement())
        .build();
  }

  @Override
  public byte[] pingBinary(PingRequest pingRequest) throws TException {
    return "hello!".getBytes(StandardCharsets.UTF_8);
  }

  @Override
  public PingResponse pingException(PingRequest pingRequest) throws CustomException, TException {
    if ("npe".equals(pingRequest.getRequest())) {
      throw new NullPointerException("npe");
    }

    if ("wait".equals(pingRequest.getRequest())) {
      waitForMs(5000);
    }

    throw new CustomException.Builder()
        .setMessage("custom exception: " + pingRequest.getRequest())
        .build();
  }

  private static void waitForMs(long millis) {
    try {
      Thread.sleep(millis);
    } catch (Exception e) {
      // Suppress test exception
    }
  }

  @Override
  public void pingVoid(PingRequest pingRequest) throws TException {
    System.out.println("got -> " + pingRequest.toString());
  }
}
