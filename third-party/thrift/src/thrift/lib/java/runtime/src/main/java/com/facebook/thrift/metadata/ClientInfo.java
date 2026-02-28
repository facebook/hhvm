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

package com.facebook.thrift.metadata;

import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

public class ClientInfo {

  public static final String CLIENT_METADATA_HEADER = "client_metadata";

  private static final String FBCODE_BUILD_RULE_MANIFEST_ATTR = "Fbcode-Build-Rule";
  private static final String OTHER_METADATA = getOtherMetadata();

  private static Set<Runtime> runtimeSet = new ConcurrentSkipListSet<>();
  private static Set<Client> clientSet = new ConcurrentSkipListSet<>();
  private static Set<Transport> transportSet = new ConcurrentSkipListSet<>();
  private static AtomicInteger counter = new AtomicInteger(0);
  private static String cachedInfo = "";
  private static int cachedCounter = -1;

  public enum Runtime {
    JAVA2("java2"),
    SWIFT("swift");

    private final String value;

    private Runtime(String value) {
      this.value = value;
    }
  }

  public enum Client {
    ABSTRACT("abstract"),
    WRAPPER("wrapper");

    private final String value;

    private Client(String value) {
      this.value = value;
    }
  }

  public enum Transport {
    HEADER("header"),
    ROCKET("rocket");

    private final String value;

    private Transport(String value) {
      this.value = value;
    }
  }

  public static void addRuntime(Runtime runtime) {
    if (runtimeSet.add(runtime)) {
      counter.incrementAndGet();
    }
  }

  public static void addClient(Client client) {
    if (clientSet.add(client)) {
      counter.incrementAndGet();
    }
  }

  public static void addTransport(Transport transport) {
    if (transportSet.add(transport)) {
      counter.incrementAndGet();
    }
  }

  private static String getClientAgent() {
    int curr = counter.get();
    if (cachedCounter == curr) {
      return cachedInfo;
    }

    StringBuilder st = new StringBuilder();
    for (Runtime r : runtimeSet) {
      st.append(r.value).append(".");
    }
    for (Client c : clientSet) {
      st.append(c.value).append(".");
    }
    for (Transport t : transportSet) {
      st.append(t.value).append(".");
    }
    st.append("java");

    cachedInfo = st.toString();
    cachedCounter = curr;
    return cachedInfo;
  }

  private static String getOtherMetadata() {
    String s =
        String.format(
            "\",\"otherMetadata\":{\"build_rule\":\"%s\",\"tw_cluster\":\"%s\",\"tw_user\":\"%s\",\"tw_job\":\"%s\",\"tw_task\":\"%s\",\"tw_oncall_team\":\"%s\"}}",
            getClientBuildRule(),
            nullSafe(System.getenv("TW_JOB_CLUSTER")),
            nullSafe(System.getenv("TW_JOB_USER")),
            nullSafe(System.getenv("TW_JOB_NAME")),
            nullSafe(System.getenv("TW_TASK_ID")),
            nullSafe(System.getenv("TW_ONCALL_TEAM")));
    return s;
  }

  public static String getClientMetadata() {
    StringBuffer st = new StringBuffer(OTHER_METADATA.length() + 50);
    st.append("{\"agent\":\"");
    st.append(getClientAgent());
    st.append(OTHER_METADATA);

    return st.toString();
  }

  private static String nullSafe(String s) {
    if (s == null) {
      return "<unknown>";
    } else {
      return s;
    }
  }

  private static String getClientBuildRule() {
    try {
      Enumeration<URL> resources =
          ClientInfo.class.getClassLoader().getResources("META-INF/MANIFEST.MF");
      while (resources.hasMoreElements()) {
        Manifest manifest = new Manifest(resources.nextElement().openStream());
        Attributes attrs = manifest.getMainAttributes();

        Attributes.Name buildRuleName = new Attributes.Name(FBCODE_BUILD_RULE_MANIFEST_ATTR);
        if (attrs.containsKey(buildRuleName)) {
          return attrs.getValue(buildRuleName);
        }
      }
    } catch (IOException e) {
      return "";
    }

    return "<unknown_build_rule>";
  }

  public static void reset() {
    runtimeSet.clear();
    clientSet.clear();
    transportSet.clear();
    cachedCounter = -1;
    cachedInfo = "";
  }
}
