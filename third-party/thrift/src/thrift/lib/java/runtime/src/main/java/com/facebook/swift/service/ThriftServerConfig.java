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

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkState;
import static io.airlift.units.DataSize.Unit.MEGABYTE;
import static java.util.Objects.requireNonNull;
import static java.util.concurrent.TimeUnit.DAYS;
import static java.util.concurrent.TimeUnit.MINUTES;

import com.google.common.base.Optional;
import com.google.common.base.Splitter;
import com.google.common.collect.ImmutableList;
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import io.airlift.configuration.Config;
import io.airlift.units.DataSize;
import io.airlift.units.Duration;
import io.airlift.units.MaxDataSize;
import io.airlift.units.MinDataSize;
import io.airlift.units.MinDuration;
import io.netty.handler.ssl.ClientAuth;
import java.io.File;
import java.util.List;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;

public class ThriftServerConfig {
  private static final int DEFAULT_BOSS_THREAD_COUNT = 1;
  private static final int DEFAULT_IO_WORKER_THREAD_COUNT =
      2 * Runtime.getRuntime().availableProcessors();
  private static final int DEFAULT_WORKER_THREAD_COUNT = 200;
  private static final int DEFAULT_PER_CONNECTION_QUEUED_RESPONSE_LIMIT = 16;

  private String bindAddress = "localhost";
  private boolean bindAddressEnabled = false;
  private int port;
  private int acceptBacklog = 1024;
  private int connectionLimit;
  private int maxQueuedResponsesPerConnection = DEFAULT_PER_CONNECTION_QUEUED_RESPONSE_LIMIT;
  private int acceptorThreadCount = DEFAULT_BOSS_THREAD_COUNT;
  private int ioThreadCount = DEFAULT_IO_WORKER_THREAD_COUNT;
  private int trafficClass = 0;
  private Duration idleConnectionTimeout = Duration.valueOf("60s");
  private Duration taskExpirationTimeout = Duration.valueOf("5s");
  private Duration queueTimeout = null;
  private Optional<Integer> workerThreads = Optional.absent();
  private Optional<Integer> maxQueuedRequests = Optional.absent();
  private Optional<ExecutorService> workerExecutor = Optional.absent();
  private Optional<String> workerExecutorKey = Optional.absent();
  private String transportName = "header";
  private String protocolName = "header";

  // Netty 4 Configs
  private Duration requestTimeout = new Duration(1, MINUTES);
  private Duration sslContextRefreshTime = new Duration(1, MINUTES);
  private boolean allowPlaintext = true;
  private boolean enableJdkSsl = false;
  private List<String> ciphers = ImmutableList.of();
  private boolean sslEnabled = true;
  private File trustCertificate;
  private File key;
  private String keyPassword;
  private long sessionCacheSize = 10_000;
  private Duration sessionTimeout = new Duration(1, DAYS);
  private boolean assumeClientsSupportOutOfOrderResponses = true;
  private String certFile = "/var/facebook/x509_identities/server.pem";
  private String keyFile = "/var/facebook/x509_identities/server.pem";
  private String caFile = "/var/facebook/rootcanal/ca.pem";
  private boolean enableUDS = false;
  private String udsPath = null;
  private ClientAuth clientAuth = ClientAuth.OPTIONAL;

  private boolean enableAlpn = false;

  /**
   * The default maximum allowable size for a single incoming thrift request or outgoing thrift
   * response. A server can configure the actual maximum to be much higher (up to 0x3FFFFFFF or
   * almost 1 GB). The default max could also be safely bumped up, but 64MB is chosen simply because
   * it seems reasonable that if you are sending requests or responses larger than that, it should
   * be a conscious decision (something you must manually configure).
   */
  private DataSize maxFrameSize = new DataSize(64, MEGABYTE);

  public String getBindAddress() {
    return bindAddress;
  }

  @Config("thrift.bind-address")
  public ThriftServerConfig setBindAddress(String bindAddress) {
    this.bindAddress = bindAddress;
    return this;
  }

  public boolean isBindAddressEnabled() {
    return bindAddressEnabled;
  }

  @Config("thrift.bind-address-enabled")
  public ThriftServerConfig setBindAddressEnabled(boolean bindAddressEnabled) {
    this.bindAddressEnabled = bindAddressEnabled;
    return this;
  }

  @Min(0)
  @Max(65535)
  public int getPort() {
    return port;
  }

  @Config("thrift.port")
  public ThriftServerConfig setPort(int port) {
    this.port = port;
    return this;
  }

  @MinDataSize("0B")
  // 0x3FFFFFFF bytes
  @MaxDataSize("1073741823B")
  public DataSize getMaxFrameSize() {
    return maxFrameSize;
  }

  /** Sets a maximum frame size */
  @Config("thrift.max-frame-size")
  public ThriftServerConfig setMaxFrameSize(DataSize maxFrameSize) {
    checkArgument(maxFrameSize.toBytes() <= 0x3FFFFFFF);
    this.maxFrameSize = maxFrameSize;
    return this;
  }

  /**
   * Sets the number of pending connections that the {@link java.net.ServerSocket} will queue up
   * before the server process can actually accept them. If your server may take a lot of
   * connections in a very short interval, you'll want to set this higher to avoid rejecting some of
   * the connections. Setting this to 0 will apply an implementation-specific default.
   *
   * <p>
   *
   * <p>The default value is 1024.
   *
   * <p>
   *
   * <p>Actual behavior of the socket backlog is dependent on OS and JDK implementation, and it may
   * even be ignored on some systems. See JDK docs <a
   * href="http://docs.oracle.com/javase/7/docs/api/java/net/ServerSocket.html#ServerSocket%28int%2C%20int%29"
   * target="_top">here</a> for details.
   */
  @Config("thrift.accept-backlog")
  public ThriftServerConfig setAcceptBacklog(int acceptBacklog) {
    this.acceptBacklog = acceptBacklog;
    return this;
  }

  @Min(0)
  public int getAcceptBacklog() {
    return acceptBacklog;
  }

  public int getAcceptorThreadCount() {
    return acceptorThreadCount;
  }

  @Config("thrift.acceptor-threads.count")
  public ThriftServerConfig setAcceptorThreadCount(int acceptorThreadCount) {
    this.acceptorThreadCount = acceptorThreadCount;
    return this;
  }

  public int getIoThreadCount() {
    return ioThreadCount;
  }

  @Config("thrift.io-threads.count")
  public ThriftServerConfig setIoThreadCount(int ioThreadCount) {
    this.ioThreadCount = ioThreadCount;
    return this;
  }

  public int getTrafficClass() {
    return trafficClass;
  }

  @Config("thrift.traffic-class")
  public ThriftServerConfig setTrafficClass(int trafficClass) {
    this.trafficClass = trafficClass;
    return this;
  }

  public Duration getIdleConnectionTimeout() {
    return this.idleConnectionTimeout;
  }

  /**
   * Sets a timeout period between receiving requests from a client connection. If the timeout is
   * exceeded (no complete requests have arrived from the client within the timeout), the server
   * will disconnect the idle client.
   *
   * <p>The default is 60s.
   *
   * @param idleConnectionTimeout The timeout
   * @return This {@link ThriftServerConfig} instance
   */
  @Config("thrift.idle-connection-timeout")
  public ThriftServerConfig setIdleConnectionTimeout(Duration idleConnectionTimeout) {
    this.idleConnectionTimeout = idleConnectionTimeout;
    return this;
  }

  public Duration getQueueTimeout() {
    return queueTimeout;
  }

  /**
   * Sets a timeout period between receiving a request and the pulling the request off the queue. If
   * the timeout expires before the request reaches the front of the queue and begins processing,
   * the server will discard the request instead of processing it.
   *
   * @param queueTimeout The timeout
   * @return This {@link ThriftServerConfig} instance
   */
  @Config("thrift.queue-timeout")
  public ThriftServerConfig setQueueTimeout(Duration queueTimeout) {
    this.queueTimeout = queueTimeout;
    return this;
  }

  public Duration getTaskExpirationTimeout() {
    return taskExpirationTimeout;
  }

  /**
   * Sets a timeout period between receiving a request and the completion of that request. If the
   * timeout expires before the request reaches the front of the queue and begins processing, the
   * server will discard the request instead of processing it. If the timeout expires after the
   * request has started processing, the server will send an error immediately, and discard the
   * result of request handling.
   *
   * @param taskExpirationTimeout The timeout
   * @return This {@link ThriftServerConfig} instance
   */
  @Config("thrift.task-expiration-timeout")
  public ThriftServerConfig setTaskExpirationTimeout(Duration taskExpirationTimeout) {
    this.taskExpirationTimeout = taskExpirationTimeout;
    return this;
  }

  @Min(0)
  public int getConnectionLimit() {
    return this.connectionLimit;
  }

  /**
   * Sets an upper bound on the number of concurrent connections the server will accept.
   *
   * <p>The default is not to limit the number of connections.
   *
   * @param connectionLimit The maximum number of concurrent connections
   * @return This {@link ThriftServerConfig} instance
   */
  @Config("thrift.connection-limit")
  public ThriftServerConfig setConnectionLimit(int connectionLimit) {
    this.connectionLimit = connectionLimit;
    return this;
  }

  @Min(1)
  public int getWorkerThreads() {
    return workerThreads.or(DEFAULT_WORKER_THREAD_COUNT);
  }

  /**
   * Sets the number of worker threads that will be created for processing thrift requests after
   * they have arrived. Any value passed here will be ignored if {@link
   * ThriftServerConfig#setWorkerExecutor(ExecutorService)} is called.
   *
   * <p>The default value is 200.
   *
   * @param workerThreads Number of worker threads to use
   * @return This {@link ThriftServerConfig} instance
   */
  @Config("thrift.threads.max")
  public ThriftServerConfig setWorkerThreads(int workerThreads) {
    this.workerThreads = Optional.of(workerThreads);
    return this;
  }

  public String getWorkerExecutorKey() {
    return workerExecutorKey.orNull();
  }

  /**
   * Sets the key for locating an {@link ExecutorService} from the mapped executors installed by
   * Guice modules.
   *
   * <p>If you are not configuring your application using Guice, it will probably be simpler to just
   * call {@link ThriftServerConfig#setWorkerExecutor(ExecutorService)} instead.
   *
   * <p>Use of this method on a {@link ThriftServerConfig} instance is incompatible with use of
   * {@link ThriftServerConfig#setWorkerExecutor(ExecutorService)} or {@link
   * ThriftServerConfig#setWorkerThreads(int)}
   */
  @Config("thrift.worker-executor-key")
  public ThriftServerConfig setWorkerExecutorKey(String workerExecutorKey) {
    this.workerExecutorKey = Optional.fromNullable(workerExecutorKey);
    return this;
  }

  public Integer getMaxQueuedRequests() {
    return maxQueuedRequests.orNull();
  }

  /**
   * Sets the maximum number of received requests that will wait in the queue to be executed.
   *
   * <p>After this many requests are waiting, the worker queue will start rejecting requests, which
   * will cause the server to fail those requests.
   */
  @Config("thrift.max-queued-requests")
  public ThriftServerConfig setMaxQueuedRequests(Integer maxQueuedRequests) {
    this.maxQueuedRequests = Optional.fromNullable(maxQueuedRequests);
    return this;
  }

  public int getMaxQueuedResponsesPerConnection() {
    return maxQueuedResponsesPerConnection;
  }

  /**
   * Sets the maximum number of responses that may accumulate per connection before the connection
   * starts blocking reads (to avoid building up limitless queued responses).
   *
   * <p>This limit applies whenever either the client doesn't support receiving out-of-order
   * responses.
   */
  @Config("thrift.max-queued-responses-per-connection")
  public ThriftServerConfig setMaxQueuedResponsesPerConnection(
      int maxQueuedResponsesPerConnection) {
    this.maxQueuedResponsesPerConnection = maxQueuedResponsesPerConnection;
    return this;
  }

  /**
   * Builds the {@link ExecutorService} used for running Thrift server methods.
   *
   * <p>
   *
   * <p>The details of the {@link ExecutorService} that gets built can be tweaked by calling any of
   * the following (though only <b>one</b> of these should actually be called):
   *
   * <p>
   *
   * <ul>
   *   <li>{@link ThriftServerConfig#setWorkerThreads}
   *   <li>{@link ThriftServerConfig#setWorkerExecutor}
   *   <li>{@link ThriftServerConfig#setWorkerExecutorKey}
   * </ul>
   *
   * <p>
   *
   * <p>The default behavior if none of the above were called is to synthesize a fixed-size {@link
   * ThreadPoolExecutor} using {@link ThriftServerConfig#DEFAULT_WORKER_THREAD_COUNT} threads.
   */
  public ExecutorService getOrBuildWorkerExecutor(
      Map<String, ExecutorService> boundWorkerExecutors) {
    if (workerExecutorKey.isPresent()) {
      checkState(
          !workerExecutor.isPresent(),
          "Worker executor key should not be set along with a specific worker executor instance");
      checkState(
          !workerThreads.isPresent(),
          "Worker executor key should not be set along with a number of worker threads");
      checkState(
          !maxQueuedRequests.isPresent(),
          "When using a custom executor, handling maximum queued requests must be done manually");

      String key = workerExecutorKey.get();
      checkArgument(
          boundWorkerExecutors.containsKey(key),
          "No ExecutorService was bound to key '" + key + "'");
      ExecutorService executor = boundWorkerExecutors.get(key);
      checkNotNull(executor, "WorkerExecutorKey maps to null");
      return executor;
    } else if (workerExecutor.isPresent()) {
      checkState(
          !workerThreads.isPresent(),
          "Worker executor should not be set along with number of worker threads");
      checkState(
          !maxQueuedRequests.isPresent(),
          "When using a custom executor, handling maximum queued requests must be done manually");

      return workerExecutor.get();
    } else {
      return makeDefaultWorkerExecutor();
    }
  }

  /**
   * Sets the executor that will be used to process thrift requests after they arrive. Setting this
   * will override any call to {@link ThriftServerConfig#setWorkerThreads(int)}.
   *
   * <p>Use of this method on a {@link ThriftServerConfig} instance is incompatible with use of
   * {@link ThriftServerConfig#setWorkerExecutorKey(String)} or {@link
   * ThriftServerConfig#setWorkerThreads(int)}
   *
   * @param workerExecutor The worker executor
   * @return This {@link ThriftServerConfig} instance
   */
  public ThriftServerConfig setWorkerExecutor(ExecutorService workerExecutor) {
    this.workerExecutor = Optional.of(workerExecutor);
    return this;
  }

  private ExecutorService makeDefaultWorkerExecutor() {
    BlockingQueue<Runnable> queue;

    if (maxQueuedRequests.isPresent()) {
      // Create a limited-capacity executor that will throw RejectedExecutionException when full.
      // NiftyDispatcher will handle RejectedExecutionException by sending a TApplicationException.
      queue = new LinkedBlockingQueue<>(maxQueuedRequests.get());
    } else {
      queue = new LinkedBlockingQueue<>();
    }

    return new ThreadPoolExecutor(
        getWorkerThreads(),
        getWorkerThreads(),
        0L,
        TimeUnit.MILLISECONDS,
        queue,
        new ThreadFactoryBuilder().setNameFormat("thrift-worker-%s").build(),
        new ThreadPoolExecutor.AbortPolicy());
  }

  /**
   * Sets the name of the transport (frame codec) that this server will handle. The available
   * options by default are 'buffered', and 'framed'. Additional modules may install other options.
   * Server startup will fail if you specify an unavailable transport here.
   *
   * @param transportName The name of the transport
   * @return This {@link ThriftServerConfig} instance
   */
  @Config("thrift.transport")
  public ThriftServerConfig setTransportName(String transportName) {
    this.transportName = transportName;
    return this;
  }

  @NotNull
  public String getTransportName() {
    return transportName;
  }

  /**
   * Sets the name of the protocol that this server will speak. The available options by default are
   * 'binary' and 'compact'. Additional modules may install other options. Server startup will fail
   * if you specify an unavailable protocol here.
   *
   * @param protocolName The name of the protocol
   * @return This {@link ThriftServerConfig} instance
   */
  @Config("thrift.protocol")
  public ThriftServerConfig setProtocolName(String protocolName) {
    this.protocolName = protocolName;
    return this;
  }

  @NotNull
  public String getProtocolName() {
    return protocolName;
  }

  public boolean isAllowPlaintext() {
    return allowPlaintext;
  }

  @Config("thrift.server.allow-plaintext")
  public ThriftServerConfig setAllowPlaintext(boolean allowPlaintext) {
    this.allowPlaintext = allowPlaintext;
    return this;
  }

  @MinDuration("1s")
  public Duration getSslContextRefreshTime() {
    return sslContextRefreshTime;
  }

  @Config("thrift.server.ssl-context.refresh-time")
  public ThriftServerConfig setSslContextRefreshTime(Duration sslContextRefreshTime) {
    this.sslContextRefreshTime = sslContextRefreshTime;
    return this;
  }

  public boolean isSslEnabled() {
    return sslEnabled;
  }

  @Config("thrift.server.ssl.enabled")
  public ThriftServerConfig setSslEnabled(boolean sslEnabled) {
    this.sslEnabled = sslEnabled;
    return this;
  }

  public File getTrustCertificate() {
    return trustCertificate;
  }

  @Config("thrift.server.ssl.trust-certificate")
  public ThriftServerConfig setTrustCertificate(File trustCertificate) {
    this.trustCertificate = trustCertificate;
    return this;
  }

  public File getKey() {
    return key;
  }

  @Config("thrift.server.ssl.key")
  public ThriftServerConfig setKey(File key) {
    this.key = key;
    return this;
  }

  public String getKeyPassword() {
    return keyPassword;
  }

  @Config("thrift.server.ssl.key-password")
  public ThriftServerConfig setKeyPassword(String keyPassword) {
    this.keyPassword = keyPassword;
    return this;
  }

  public long getSessionCacheSize() {
    return sessionCacheSize;
  }

  @Config("thrift.server.ssl.session-cache-size")
  public ThriftServerConfig setSessionCacheSize(long sessionCacheSize) {
    this.sessionCacheSize = sessionCacheSize;
    return this;
  }

  public Duration getSessionTimeout() {
    return sessionTimeout;
  }

  @Config("thrift.server.ssl.session-timeout")
  public ThriftServerConfig setSessionTimeout(Duration sessionTimeout) {
    this.sessionTimeout = sessionTimeout;
    return this;
  }

  public List<String> getCiphers() {
    return ciphers;
  }

  @Config("thrift.server.ssl.ciphers")
  public ThriftServerConfig setCiphers(String ciphers) {
    this.ciphers =
        Splitter.on(',')
            .trimResults()
            .omitEmptyStrings()
            .splitToList(requireNonNull(ciphers, "ciphers is null"));
    return this;
  }

  public boolean isAssumeClientsSupportOutOfOrderResponses() {
    return assumeClientsSupportOutOfOrderResponses;
  }

  @Config("thrift.server.assume-clients-support-out-of-order-responses")
  public ThriftServerConfig setAssumeClientsSupportOutOfOrderResponses(
      boolean assumeClientsSupportOutOfOrderResponses) {
    this.assumeClientsSupportOutOfOrderResponses = assumeClientsSupportOutOfOrderResponses;
    return this;
  }

  @Config("thrift.cert")
  public ThriftServerConfig setCertFile(String certFile) {
    this.certFile = certFile;
    return this;
  }

  public String getCertFile() {
    return this.certFile;
  }

  @Config("thrift.key")
  public ThriftServerConfig setKeyFile(String keyFile) {
    this.keyFile = keyFile;
    return this;
  }

  public String getKeyFile() {
    return this.keyFile;
  }

  @Config("thrift.ca_file")
  public ThriftServerConfig setCAFile(String caFile) {
    this.caFile = caFile;
    return this;
  }

  public String getCAFile() {
    return this.caFile;
  }

  @Config("thrift.ssl.enabledJdkSsl")
  public ThriftServerConfig setEnableJdkSsl(boolean enableJdkSsl) {
    this.enableJdkSsl = enableJdkSsl;
    return this;
  }

  public boolean getEnableJdkSsl() {
    return enableJdkSsl;
  }

  public ClientAuth getClientAuth() {
    return clientAuth;
  }

  /**
   * Sets client authentication mode for SSL handler.
   *
   * @param clientAuth Client auth value(NONE/OPTIONAL/REQUIRE)
   * @return Instance of ThriftServerConfig
   */
  @Config("thrift.ssl.clientAuth")
  public ThriftServerConfig setClientAuth(ClientAuth clientAuth) {
    this.clientAuth = clientAuth;
    return this;
  }

  /**
   * Returns whether server is using UDS.
   *
   * @return
   */
  public boolean isEnableUDS() {
    return enableUDS;
  }

  /**
   * Enables Server using UDS instead of TCP. Also, Sets socket file path for UDS servers. If the
   * folder to socket file does not exist or it is an invalid path, server initialization will fail.
   *
   * @param udsPath: Non-null parameter path
   * @return Instance of ThriftServerConfig
   */
  @Config("thrift.uds-path")
  public ThriftServerConfig setUdsPath(String udsPath) {
    this.udsPath = udsPath;
    this.enableUDS = udsPath != null;
    return this;
  }

  /**
   * Returns socket file path for UDS servers
   *
   * @return: UDS Path
   */
  public String getUdsPath() {
    return udsPath;
  }

  @Config("thrift.enable-alpn")
  public ThriftServerConfig setEnableAlpn(boolean enableAlpn) {
    this.enableAlpn = enableAlpn;
    return this;
  }

  public boolean isEnableAlpn() {
    return enableAlpn;
  }

  @Override
  public String toString() {
    return "ThriftServerConfig{"
        + "bindAddress='"
        + bindAddress
        + '\''
        + ", port="
        + port
        + ", acceptBacklog="
        + acceptBacklog
        + ", connectionLimit="
        + connectionLimit
        + ", maxQueuedResponsesPerConnection="
        + maxQueuedResponsesPerConnection
        + ", acceptorThreadCount="
        + acceptorThreadCount
        + ", ioThreadCount="
        + ioThreadCount
        + ", trafficClass="
        + trafficClass
        + ", idleConnectionTimeout="
        + idleConnectionTimeout
        + ", taskExpirationTimeout="
        + taskExpirationTimeout
        + ", queueTimeout="
        + queueTimeout
        + ", workerThreads="
        + workerThreads
        + ", maxQueuedRequests="
        + maxQueuedRequests
        + ", workerExecutor="
        + workerExecutor
        + ", workerExecutorKey="
        + workerExecutorKey
        + ", transportName='"
        + transportName
        + '\''
        + ", protocolName='"
        + protocolName
        + '\''
        + ", requestTimeout="
        + requestTimeout
        + ", sslContextRefreshTime="
        + sslContextRefreshTime
        + ", allowPlaintext="
        + allowPlaintext
        + ", enableJdkSsl="
        + enableJdkSsl
        + ", udsPath='"
        + udsPath
        + ", ciphers="
        + ciphers
        + ", sslEnabled="
        + sslEnabled
        + ", keyPassword='"
        + keyPassword
        + '\''
        + ", sessionCacheSize="
        + sessionCacheSize
        + ", sessionTimeout="
        + sessionTimeout
        + ", assumeClientsSupportOutOfOrderResponses="
        + assumeClientsSupportOutOfOrderResponses
        + ", certFile='"
        + certFile
        + '\''
        + ", keyFile='"
        + keyFile
        + '\''
        + ", caFile='"
        + caFile
        + ", enableAlpn='"
        + enableAlpn
        + '\''
        + '}';
  }
}
