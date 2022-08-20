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

package com.meta.thrift.conformance.server;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.ServerTransport;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.TransportType;
import java.net.InetSocketAddress;
import org.apache.thrift.conformance.ConformanceService;

public class ConformanceServer {

  public static void main(String[] args) {
    RpcServerHandler handler =
        ConformanceService.serverHandlerBuilder(new ConformanceServiceImpl()).build();

    ServerTransport transport =
        RpcServerUtils.createServerTransport(
                new ThriftServerConfig().setPort(0), TransportType.THEADER, handler)
            .block();

    // Conformance test runner starts multiple instances of server
    // and reads the port from std output
    // Ports should be allocated automatically
    System.out.println(((InetSocketAddress) transport.getAddress()).getPort());

    transport.onClose().block();
  }
}
