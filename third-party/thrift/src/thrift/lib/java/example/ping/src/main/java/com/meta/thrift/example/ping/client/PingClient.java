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

package com.meta.thrift.example.ping.client;

import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.legacy.client.LegacyRpcClientFactory;
import com.facebook.thrift.rsocket.client.RSocketRpcClientFactory;
import com.google.common.base.Charsets;
import com.google.common.collect.ImmutableMap;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.thrift.ProtocolId;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class PingClient {
  private static final Logger LOG = LoggerFactory.getLogger(PingClient.class);

  private static PingClientConfig parseArgs(String[] args) {
    Options options = new Options();

    Option hostOption =
        Option.builder("h")
            .longOpt("host")
            .desc("host of PingService server (default to localhost)")
            .type(String.class)
            .hasArg(true)
            .numberOfArgs(1)
            .required(false)
            .build();
    options.addOption(hostOption);

    Option portOption =
        Option.builder("p")
            .longOpt("port")
            .desc("port of PingService server (default to 7777)")
            .type(Integer.class)
            .hasArg(true)
            .numberOfArgs(1)
            .required(false)
            .build();
    options.addOption(portOption);

    Option transportOption =
        Option.builder("t")
            .longOpt("transport")
            .desc("thrift transport (header or rsocket, default to header)")
            .type(String.class)
            .hasArg(true)
            .numberOfArgs(1)
            .required(false)
            .build();
    options.addOption(transportOption);

    Option thriftMethodOption =
        Option.builder("m")
            .longOpt("method")
            .desc("thrift method (ping, pingException or pingVoid, default to ping)")
            .type(String.class)
            .hasArg(true)
            .numberOfArgs(1)
            .required(false)
            .build();
    options.addOption(thriftMethodOption);

    CommandLineParser parser = new DefaultParser();
    HelpFormatter formatter = new HelpFormatter();
    CommandLine cmd;

    try {
      cmd = parser.parse(options, args);
    } catch (ParseException e) {
      LOG.error("Error parsing args", e);
      formatter.printHelp("scribe_cat", options);

      System.exit(1);
      return null;
    }

    PingClientConfig.Builder configBuilder = new PingClientConfig.Builder();

    if (cmd.hasOption("host")) {
      configBuilder.setHost(cmd.getOptionValue("host"));
    }
    if (cmd.hasOption("port")) {
      configBuilder.setPort(Integer.parseInt(cmd.getOptionValue("port")));
    }
    if (cmd.hasOption("transport")) {
      configBuilder.setTransport(cmd.getOptionValue("transport"));
    }
    if (cmd.hasOption("method")) {
      configBuilder.setMethod(cmd.getOptionValue("method"));
    }

    return configBuilder.build();
  }

  public static void main(String[] args) {
    PingClientConfig config = parseArgs(args);

    boolean useHeader = "header".equals(config.getTransport());
    final RpcClientFactory clientFactory =
        useHeader
            ? new LegacyRpcClientFactory(new ThriftClientConfig().setDisableSSL(true))
            : new RSocketRpcClientFactory(new ThriftClientConfig().setDisableSSL(true));
    SocketAddress address = InetSocketAddress.createUnresolved(config.getHost(), config.getPort());

    // NOTE: the follow code can be simplified after a better api is introduced
    final PingService client =
        PingService.clientBuilder()
            .setProtocolId(ProtocolId.BINARY)
            .setHeaders(ImmutableMap.of("key1", "val1"))
            .setPersistentHeaders(ImmutableMap.of("pkey1", "pval1"))
            .build(clientFactory, address);

    // Create request object
    PingRequest request = new PingRequest("Foo");
    try {
      // Send request
      if ("ping".equals(config.getMethod())) {
        PingResponse response = client.ping(request);
        LOG.info("Response: " + response.getResponse());
      } else if ("pingBinary".equals(config.getMethod())) {
        LOG.info("calling pingBinary");
        byte[] response = client.pingBinary(request);
        LOG.info("response length = " + response.length);
        LOG.info("response: " + new String(response, Charsets.UTF_8));
      } else if ("pingException".equals(config.getMethod())) {
        PingResponse response = client.ping(request);
        LOG.info("Response: " + response.getResponse());
      } else if ("pingVoid".equals(config.getMethod())) {
        client.pingVoid(request);
      }
    } catch (Exception ex) {
      LOG.error("Exception: " + ex);
    }
  }
}
