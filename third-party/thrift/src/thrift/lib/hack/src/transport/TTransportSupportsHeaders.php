<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
 * @package thrift.transport
 */

interface TTransportSupportsHeaders {
  public function setHeader(string $str_key, @string $str_value): this;
  public function setPersistentHeader(
    string $str_key,
    @string $str_value,
  ): this;
  public function getWriteHeaders(): KeyedContainer<string, string>;
  public function getPersistentWriteHeaders(): KeyedContainer<string, string>;
  public function getHeaders(): KeyedContainer<string, string>;
  public function clearHeaders(): void;
  public function clearPersistentHeaders(): void;
}
