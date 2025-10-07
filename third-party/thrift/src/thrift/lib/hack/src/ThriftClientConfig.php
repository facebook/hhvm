<?hh
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
 *
 */

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

// Use this to control hack language specific configs for Thrift Client
final class ThriftClientConfig {
  /**
   * For streaming, default behavior is to send more credits when client has
   * consumed either chunkBufferSize/2 payloads or 16KB bytes.
   *
   * When all payloads are close to 16KB, this means credit is sent after
   * every payload. In such cases, clients can choose to disable 16KB limit
   * and only send credits after chunkBufferSize/2 payloads are consumed.
   */
  private bool $streamDisable16KBLimit = false;
  public function __construct()[] {}

  public function setStreamDisable16KBLimit(bool $val)[write_props]: this {
    $this->streamDisable16KBLimit = $val;
    return $this;
  }

  public function getStreamDisable16KBLimit()[]: bool {
    return $this->streamDisable16KBLimit;
  }
}
