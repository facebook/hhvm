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

package com.facebook.thrift.type;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import java.util.Objects;
import java.util.regex.Pattern;

public class UniversalName {

  private static final String DOMAIN_NAME_PATTERN = "(([a-z0-9-]{1,})\\.)+[a-z0-9-]{1,}/";
  private static final String PACKAGE_NAME_PATTERN = "(([a-zA-Z0-9_-]{1,})/)+";
  private static final String TYPE_PATTERN = "[a-zA-Z0-9_-]{1,}";
  private static final String UNIVERSAL_NAME_PATTERN =
      DOMAIN_NAME_PATTERN + PACKAGE_NAME_PATTERN + TYPE_PATTERN;

  private static Pattern namePattern;

  static {
    namePattern = Pattern.compile(UNIVERSAL_NAME_PATTERN);
  }

  private final HashAlgorithm algorithm;
  private final String uri;
  private final ByteBuf hash;

  /**
   * Creates a new universal name by given uri and hash algorithm. Given uri is validated based on
   * the rules defined. When the thrift compiler validates all uri's in thrift IDL file, this
   * validation might be lifted.
   *
   * @param uri uri of the universal name.
   * @param algorithm Hash algorithm.
   * @throws InvalidUniversalNameURIException If the uri syntax is invalid.
   */
  public UniversalName(String uri, HashAlgorithm algorithm) {
    if (!namePattern.matcher(uri).matches()) {
      throw new InvalidUniversalNameURIException(uri);
    }
    this.uri = Objects.requireNonNull(uri, "Uri must not be null");
    this.algorithm = Objects.requireNonNull(algorithm, "Algorithm must not be null");
    this.hash = algorithm.generateHash(uri);
  }

  /**
   * Creates new universal name object with the SHA-256 algorithm.
   *
   * @param uri uri of the universal name.
   * @throws InvalidUniversalNameURIException If the uri syntax is invalid.
   */
  public UniversalName(String uri) {
    this(uri, HashAlgorithmSHA256.INSTANCE);
  }

  public String getHash() {
    return getHashPrefix(this.algorithm.getDefaultHashBytes());
  }

  public ByteBuf getHashBytes() {
    return hash;
  }

  public String getHashPrefix(int size) {
    return ByteBufUtil.hexDump(
        this.hash,
        0,
        Math.min(
            Math.max(size, this.algorithm.getMinHashBytes()),
            this.algorithm.getDefaultHashBytes()));
  }

  public ByteBuf getHashPrefixBytes(int size) {
    return this.hash.copy(
        0,
        Math.min(
            Math.max(size, this.algorithm.getMinHashBytes()),
            this.algorithm.getDefaultHashBytes()));
  }

  public String getUri() {
    return this.uri;
  }

  public boolean preferHash() {
    return this.algorithm.getDefaultHashBytes() < this.uri.length();
  }

  public boolean preferHash(int hashSize) {
    return Math.min(
            this.algorithm.getDefaultHashBytes(),
            Math.max(hashSize, this.algorithm.getMinHashBytes()))
        < this.uri.length();
  }

  @Override
  public String toString() {
    return "UniversalName{"
        + "algorithm="
        + algorithm
        + ", uri='"
        + uri
        + '\''
        + ", hash="
        + getHash()
        + '}';
  }
}
