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

import io.netty.handler.ssl.util.SelfSignedCertificate;
import java.io.IOException;
import java.security.cert.CertificateException;

/**
 * Utility class for generating self-signed certificates for testing using Netty.
 *
 * <p>This creates self-signed certificates on demand using Netty's SelfSignedCertificate that can
 * be used with ThriftServerConfig's keyFile, certFile, and caFile properties.
 */
public class TestCertificateUtil {
  private static SelfSignedCertificate certificate;

  /**
   * Initializes self-signed certificate using Netty.
   *
   * @throws CertificateException if certificate generation fails
   * @throws IOException if file writing fails
   */
  public static synchronized void initialize() throws CertificateException, IOException {
    if (certificate == null) {
      // Generate self-signed certificate - Netty will create temporary PEM files
      certificate = new SelfSignedCertificate();
    }
  }

  /**
   * Gets the path to the private key file.
   *
   * @return path to key file
   */
  public static String getKeyFilePath() {
    return certificate != null ? certificate.privateKey().getAbsolutePath() : null;
  }

  /**
   * Gets the path to the certificate file.
   *
   * @return path to cert file
   */
  public static String getCertFilePath() {
    return certificate != null ? certificate.certificate().getAbsolutePath() : null;
  }

  /**
   * Gets the path to the CA file (same as cert file for self-signed).
   *
   * @return path to CA file
   */
  public static String getCAFilePath() {
    return certificate != null ? certificate.certificate().getAbsolutePath() : null;
  }

  /** Cleans up certificate resources. */
  public static synchronized void cleanup() {
    if (certificate != null) {
      certificate.delete();
      certificate = null;
    }
  }
}
