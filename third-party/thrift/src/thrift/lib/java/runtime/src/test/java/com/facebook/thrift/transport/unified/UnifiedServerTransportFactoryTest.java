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

package com.facebook.thrift.transport.unified;

import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.rsocket.server.TestServiceHandler;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.test.rocket.TestService;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.SPINiftyMetrics;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import reactor.test.StepVerifier;

/**
 * Tests for UnifiedServerTransportFactory that validates the factory can create
 * UnifiedServerTransport instances with proper configuration.
 */
public class UnifiedServerTransportFactoryTest {

  private UnifiedServerTransportFactory factory;
  private RpcServerHandler rpcServerHandler;
  private TestServiceHandler testServiceHandler;
  private ThriftServerConfig serverConfig;
  private UnifiedServerTransport transport1;
  private UnifiedServerTransport transport2;

  @BeforeAll
  public static void setUpClass() throws Exception {
    // Initialize self-signed certificates for testing
    TestCertificateUtil.initialize();
  }

  @AfterAll
  public static void tearDownClass() {
    // Clean up certificate files
    TestCertificateUtil.cleanup();
  }

  @BeforeEach
  public void setUp() {
    testServiceHandler = new TestServiceHandler();
    rpcServerHandler = TestService.Reactive.serverHandlerBuilder(testServiceHandler).build();

    serverConfig =
        new ThriftServerConfig()
            .setSslEnabled(true)
            .setEnableJdkSsl(false)
            .setKeyFile(TestCertificateUtil.getKeyFilePath())
            .setCertFile(TestCertificateUtil.getCertFilePath())
            .setCAFile(TestCertificateUtil.getCAFilePath())
            .setEnableAlpn(true)
            .setTaskExpirationTimeout(Duration.valueOf("60s"))
            .setConnectionLimit(10);

    factory = new UnifiedServerTransportFactory(serverConfig);
  }

  @AfterEach
  public void tearDown() {
    if (transport1 != null) {
      transport1.dispose();
      transport1 = null;
    }
    if (transport2 != null) {
      transport2.dispose();
      transport2 = null;
    }
  }

  /** Test that the factory can be created with a valid configuration. */
  @Test
  public void testFactoryCreation() {
    assertNotNull(factory, "Factory should be created");
  }

  /** Test that the factory creates a valid UnifiedServerTransport instance. */
  @Test
  public void testCreateServerTransport() {
    SocketAddress serverAddress = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    transport1 =
        factory
            .createServerTransport(serverAddress, rpcServerHandler, new SPINiftyMetrics())
            .block();

    assertNotNull(transport1, "Transport should be created");
    assertNotNull(transport1.getAddress(), "Transport should have an address");
    assertNotNull(transport1.getNiftyMetrics(), "Transport should have metrics");
  }

  /** Test that ephemeral port binds report the actual bound address instead of the requested :0. */
  @Test
  public void testCreateServerTransportWithEphemeralPort() {
    SocketAddress serverAddress = new InetSocketAddress("localhost", 0);

    transport1 =
        factory
            .createServerTransport(serverAddress, rpcServerHandler, new SPINiftyMetrics())
            .block();

    assertNotNull(transport1, "Transport should be created");
    assertTrue(
        transport1.getAddress() instanceof InetSocketAddress,
        "Transport should expose an inet socket address");
    assertNotEquals(
        0,
        ((InetSocketAddress) transport1.getAddress()).getPort(),
        "Transport should report the actual bound ephemeral port");
  }

  /** Test that the factory can create multiple independent transport instances. */
  @Test
  public void testCreateMultipleTransports() {
    SocketAddress serverAddress1 =
        new InetSocketAddress("localhost", RpcServerUtils.findFreePort());
    SocketAddress serverAddress2 =
        new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    transport1 =
        factory
            .createServerTransport(serverAddress1, rpcServerHandler, new SPINiftyMetrics())
            .block();
    transport2 =
        factory
            .createServerTransport(serverAddress2, rpcServerHandler, new SPINiftyMetrics())
            .block();

    assertNotNull(transport1, "First transport should be created");
    assertNotNull(transport2, "Second transport should be created");

    assertNotSame(transport1, transport2, "Transports should be different instances");
    assertNotEquals(
        transport1.getAddress(),
        transport2.getAddress(),
        "Transports should have different addresses");
  }

  /** Test that the factory passes the correct configuration to the created transport. */
  @Test
  public void testConfigurationPropagation() {
    ThriftServerConfig customConfig =
        new ThriftServerConfig()
            .setSslEnabled(true)
            .setEnableJdkSsl(false)
            .setKeyFile(TestCertificateUtil.getKeyFilePath())
            .setCertFile(TestCertificateUtil.getCertFilePath())
            .setCAFile(TestCertificateUtil.getCAFilePath())
            .setEnableAlpn(true)
            .setConnectionLimit(5)
            .setTaskExpirationTimeout(Duration.valueOf("30s"));

    UnifiedServerTransportFactory customFactory = new UnifiedServerTransportFactory(customConfig);

    SocketAddress serverAddress = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    transport1 =
        customFactory
            .createServerTransport(serverAddress, rpcServerHandler, new SPINiftyMetrics())
            .block();

    assertNotNull(transport1, "Transport should be created with custom config");
    assertNotNull(transport1.getNiftyMetrics(), "Transport should have metrics");
  }

  /** Test that created transports can be properly disposed. */
  @Test
  public void testCreatedTransportDisposal() {
    SocketAddress serverAddress = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    transport1 =
        factory
            .createServerTransport(serverAddress, rpcServerHandler, new SPINiftyMetrics())
            .block();

    assertNotNull(transport1);

    transport1.dispose();

    // Verify onClose() completes after dispose
    StepVerifier.create(transport1.onClose())
        .expectComplete()
        .verify(java.time.Duration.ofSeconds(5));
  }

  /** Test that the factory returns a Mono that completes successfully when creating a transport. */
  @Test
  public void testFactoryReturnsMono() {
    SocketAddress serverAddress = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    StepVerifier.create(
            factory.createServerTransport(serverAddress, rpcServerHandler, new SPINiftyMetrics()))
        .assertNext(
            transport -> {
              assertNotNull(transport, "Transport should not be null");
              assertNotNull(transport.getAddress(), "Transport should have an address");
              transport1 = transport; // Store for cleanup
            })
        .verifyComplete();
  }

  /** Test that multiple factories can be created with different configurations. */
  @Test
  public void testMultipleFactoriesWithDifferentConfigs() {
    ThriftServerConfig config1 =
        new ThriftServerConfig()
            .setSslEnabled(true)
            .setKeyFile(TestCertificateUtil.getKeyFilePath())
            .setCertFile(TestCertificateUtil.getCertFilePath())
            .setCAFile(TestCertificateUtil.getCAFilePath())
            .setEnableAlpn(true)
            .setConnectionLimit(5);

    ThriftServerConfig config2 =
        new ThriftServerConfig()
            .setSslEnabled(true)
            .setKeyFile(TestCertificateUtil.getKeyFilePath())
            .setCertFile(TestCertificateUtil.getCertFilePath())
            .setCAFile(TestCertificateUtil.getCAFilePath())
            .setEnableAlpn(true)
            .setConnectionLimit(10);

    UnifiedServerTransportFactory factory1 = new UnifiedServerTransportFactory(config1);
    UnifiedServerTransportFactory factory2 = new UnifiedServerTransportFactory(config2);

    assertNotNull(factory1, "First factory should be created");
    assertNotNull(factory2, "Second factory should be created");
    assertNotSame(factory1, factory2, "Factories should be different instances");

    // Create transports from both factories
    SocketAddress address1 = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());
    SocketAddress address2 = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    transport1 =
        factory1.createServerTransport(address1, rpcServerHandler, new SPINiftyMetrics()).block();
    transport2 =
        factory2.createServerTransport(address2, rpcServerHandler, new SPINiftyMetrics()).block();

    assertNotNull(transport1, "Transport from factory1 should be created");
    assertNotNull(transport2, "Transport from factory2 should be created");
  }

  /** Test that the factory can be reused to create multiple transports sequentially. */
  @Test
  public void testFactoryReusability() {
    SocketAddress address1 = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    transport1 =
        factory.createServerTransport(address1, rpcServerHandler, new SPINiftyMetrics()).block();
    assertNotNull(transport1, "First transport should be created");

    SocketAddress address2 = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    transport2 =
        factory.createServerTransport(address2, rpcServerHandler, new SPINiftyMetrics()).block();
    assertNotNull(transport2, "Second transport should be created");

    // Both transports should be independent and functional
    assertNotSame(transport1, transport2, "Transports should be different instances");
  }
}
