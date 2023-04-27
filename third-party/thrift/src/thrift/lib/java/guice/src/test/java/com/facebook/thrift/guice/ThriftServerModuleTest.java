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

package com.facebook.thrift.guice;

import static org.junit.Assert.assertNotNull;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.swift.service.stats.ServerStats;
import com.facebook.swift.service.stats.ThriftServerStatsHandler;
import com.facebook.thrift.example.ping.PingService;
import com.google.inject.Guice;
import com.google.inject.Injector;
import com.google.inject.util.Modules;
import org.junit.Test;

public class ThriftServerModuleTest {
  @Test
  public void testThriftServiceModule() {
    Injector injector =
        Guice.createInjector(
            Modules.override(
                    new ThriftServerModule(),
                    binder ->
                        ThriftServiceExporter.binder(binder)
                            .addEventHandler(new ThriftServerStatsHandler(new ServerStats()))
                            .addServerHandler(
                                PingService.serverHandlerBuilder(new BlockingPingService())))
                .with(
                    binder -> {
                      ThriftServerConfig config = new ThriftServerConfig();
                      config.setPort(0);
                      binder.bind(ThriftServerConfig.class).toInstance(config);
                    }));

    PingService pingService = injector.getInstance(BlockingPingService.class);
    assertNotNull(pingService);
    injector.getInstance(ServerTransportSingleton.class);
  }
}
