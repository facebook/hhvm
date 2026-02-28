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

package com.facebook.nifty.ssl;

import java.security.cert.X509Certificate;

@Deprecated
public class SslSession {
  private final String alpn;
  private final String npn;
  private final String version;
  private final String cipher;
  // Time at which the connection was established since epoch in seconds.
  private final long establishedTime;
  private final X509Certificate peerCert;

  public SslSession(
      String alpn,
      String npn,
      String version,
      String cipher,
      long establishedTime,
      X509Certificate peerCert) {
    this.alpn = alpn;
    this.npn = npn;
    this.version = version;
    this.cipher = cipher;
    this.establishedTime = establishedTime;
    this.peerCert = peerCert;
  }

  public String getAlpn() {
    return alpn;
  }

  public String getNpn() {
    return npn;
  }

  public String getVersion() {
    return version;
  }

  public String getCipher() {
    return cipher;
  }

  public long getEstablishedTime() {
    return establishedTime;
  }

  public X509Certificate getPeerCert() {
    return peerCert;
  }

  public String toString() {
    return "SslSession(alpn="
        + alpn
        + ", npn="
        + npn
        + ", version="
        + version
        + ", cipher="
        + cipher
        + ", establishedTime="
        + Long.toString(establishedTime)
        + ", peerCert="
        + peerCert
        + ")";
  }
}
