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

package com.meta.thrift.conformance.rpc.client;

public class RpcClient {

  private static int getPort(String[] args) {
    for (int i = 0; i < args.length; i++) {
      if ("--port".equals(args[i])) {
        return Integer.parseInt(args[i + 1]);
      }
    }
    return 0;
  }

  public static void main(String[] args) {
    // override 16MB frame size
    System.setProperty("thrift.rsocket-max-frame-size", "64000");

    RpcClientConformanceHandler handler = new RpcClientConformanceHandler(getPort(args));

    // Execute the tests
    handler.executeTests();
  }
}
