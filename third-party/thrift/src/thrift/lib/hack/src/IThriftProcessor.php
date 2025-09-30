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

/**
 * Base interface for Thrift processors
 */
<<Oncalls('thrift')>> // @oss-disable
interface IThriftProcessor {
  abstract const class<\IThriftServiceStaticMetadata> SERVICE_METADATA_CLASS;
  abstract const string THRIFT_SVC_NAME;

  public function getEventHandler()[]: TProcessorEventHandler;
  public function setEventHandler(
    TProcessorEventHandler $handler,
  )[write_props]: this;
  public function process(
    TProtocol $input,
    TProtocol $output,
    ?string $fname = null,
    ?int $rseqid = null,
  ): bool;

  public function setIsSubRequest(
    bool $is_sub_request = true,
  )[write_props]: this;

  public function isSubRequest()[]: bool;

  public function isSupportedMethod(string $fname_with_prefix)[]: bool;
}
