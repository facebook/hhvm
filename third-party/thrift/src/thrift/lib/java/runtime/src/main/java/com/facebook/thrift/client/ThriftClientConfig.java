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

package com.facebook.thrift.client;

import static io.airlift.units.DataSize.Unit.MEGABYTE;
import static java.util.concurrent.TimeUnit.MILLISECONDS;
import static java.util.concurrent.TimeUnit.MINUTES;

import com.facebook.thrift.legacy.codec.LegacyTransportType;
import io.airlift.configuration.Config;
import io.airlift.configuration.ConfigDescription;
import io.airlift.units.DataSize;
import io.airlift.units.Duration;
import io.airlift.units.MaxDataSize;
import io.airlift.units.MinDuration;
import java.net.SocketAddress;
import javax.validation.constraints.NotNull;
import org.apache.thrift.ProtocolId;

public class ThriftClientConfig {
  private LegacyTransportType transport = LegacyTransportType.HEADER;
  private ProtocolId protocol = ProtocolId.COMPACT;
  private DataSize maxFrameSize = new DataSize(16, MEGABYTE);

  private Duration connectTimeout = new Duration(5_000, MILLISECONDS);
  private Duration requestTimeout = new Duration(1, MINUTES);

  private SocketAddress socksProxy;

  // TEMPORARY Debug flag for adding ChannelDuplexDebug handler to Rsocket Client
  // Remove after debugging @jbahr 8/5/2025
  private boolean debugNettyPipeline = false;

  // ssl
  private boolean enableJdkSsl = false;
  private boolean sslDisabled;
  private String certFile = "/var/facebook/x509_identities/server.pem";
  private String keyFile = "/var/facebook/x509_identities/server.pem";
  private String caFile = "/var/facebook/rootcanal/ca.pem";
  private long sessionCacheSize = 10_000;

  @NotNull
  public LegacyTransportType getTransport() {
    return transport;
  }

  @Config("thrift.client.transport")
  public ThriftClientConfig setTransport(LegacyTransportType transport) {
    this.transport = transport;
    return this;
  }

  @NotNull
  public ProtocolId getProtocol() {
    return protocol;
  }

  @Config("thrift.client.protocol")
  public ThriftClientConfig setProtocol(ProtocolId protocol) {
    this.protocol = protocol;
    return this;
  }

  @NotNull
  @MinDuration("1ms")
  public Duration getConnectTimeout() {
    return connectTimeout;
  }

  @Config("thrift.client.connect-timeout")
  public ThriftClientConfig setConnectTimeout(Duration connectTimeout) {
    this.connectTimeout = connectTimeout;
    return this;
  }

  @NotNull
  @MinDuration("1ms")
  public Duration getRequestTimeout() {
    return requestTimeout;
  }

  @Config("thrift.client.request-timeout")
  public ThriftClientConfig setRequestTimeout(Duration requestTimeout) {
    this.requestTimeout = requestTimeout;
    return this;
  }

  public SocketAddress getSocksProxy() {
    return socksProxy;
  }

  @Config("thrift.client.socks-proxy")
  public ThriftClientConfig setSocksProxy(SocketAddress socksProxy) {
    this.socksProxy = socksProxy;
    return this;
  }

  @MaxDataSize("1023MB")
  public DataSize getMaxFrameSize() {
    return maxFrameSize;
  }

  @Config("thrift.client.max-frame-size")
  public ThriftClientConfig setMaxFrameSize(DataSize maxFrameSize) {
    this.maxFrameSize = maxFrameSize;
    return this;
  }

  @Config("thrift.client.ssl.disabled")
  @ConfigDescription("Disable SSL")
  public ThriftClientConfig setDisableSSL(boolean disableSSL) {
    this.sslDisabled = disableSSL;
    return this;
  }

  public boolean isSslDisabled() {
    return sslDisabled;
  }

  @Config("thrift.cert")
  public ThriftClientConfig setCertFile(String certFile) {
    this.certFile = certFile;
    return this;
  }

  public String getCertFile() {
    return this.certFile;
  }

  @Config("thrift.key")
  public ThriftClientConfig setKeyFile(String keyFile) {
    this.keyFile = keyFile;
    return this;
  }

  public String getKeyFile() {
    return this.keyFile;
  }

  @Config("thrift.ca_file")
  public ThriftClientConfig setCAFile(String caFile) {
    this.caFile = caFile;
    return this;
  }

  public String getCAFile() {
    return this.caFile;
  }

  @Config("thrift.ssl.enabledJdkSsl")
  public ThriftClientConfig setEnableJdkSsl(boolean enableJdkSsl) {
    this.enableJdkSsl = enableJdkSsl;
    return this;
  }

  public boolean getEnableJdkSsl() {
    return enableJdkSsl;
  }

  public long getSessionCacheSize() {
    return sessionCacheSize;
  }

  @Config("thrift.server.ssl.session-cache-size")
  public ThriftClientConfig setSessionCacheSize(long sessionCacheSize) {
    this.sessionCacheSize = sessionCacheSize;
    return this;
  }

  @Config("thrift.client.rsocket.debug-netty-pipeline")
  public ThriftClientConfig setDebugNettyPipeline(boolean debugNettyPipeline) {
    this.debugNettyPipeline = debugNettyPipeline;
    return this;
  }

  public boolean getDebugNettyPipeline() {
    return debugNettyPipeline;
  }
}
