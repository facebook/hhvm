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
 * @package thrift
 */

/**
 * Base interface for Thrift structs
 */
<<__ConsistentConstruct>>
interface IThriftStruct extends \HH\IMemoizeParam {
  const type TFieldSpec = \ThriftStructTypes::TFieldSpec;
  const type TElemSpec = \ThriftStructTypes::TElemSpec;
  const type TGenericSpec = \ThriftStructTypes::TGenericSpec;

  abstract const dict<int, this::TFieldSpec> SPEC;
  abstract const dict<string, int> FIELDMAP;
  abstract const varray<IThriftStruct> ANNOTATIONS;
  abstract const int STRUCTURAL_ID;

  <<__Rx>>
  public function __construct();
  <<__Rx>>
  public function fromShape();
  public function getName(): string;
  public function read(TProtocol $input): int;
  public function write(TProtocol $input): int;
  public static function getAllStructuredAnnotations(): \TStructAnnotations;
}
