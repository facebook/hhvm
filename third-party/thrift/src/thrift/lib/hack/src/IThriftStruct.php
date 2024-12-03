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

<<Oncalls('thrift')>> // @oss-disable
interface IThriftStructWithClearTerseFields {
  public function clearTerseFields()[write_props]: void;
}

/**
 * Base interface for Thrift structs
 */
<<__ConsistentConstruct, Oncalls('thrift')>> // @oss-disable
// @oss-enable <<__ConsistentConstruct>>
interface IThriftStruct extends \HH\IMemoizeParam {

  abstract const ThriftStructTypes::TSpec SPEC;
  abstract const dict<string, int> FIELDMAP;
  abstract const int STRUCTURAL_ID;

  abstract const type TConstructorShape;

  public function __construct()[];
  public static function withDefaultValues()[]: this;
  public function getName()[]: string;
  public function read(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: int;
  public function write(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: int;
  public static function getAllStructuredAnnotations(
  )[write_props]: \TStructAnnotations;
}

/**
 * Base interface for Thrift structs with Sync APIs
 */
<<__ConsistentConstruct, Oncalls('thrift')>> // @oss-disable
// @oss-enable <<__ConsistentConstruct>>
interface IThriftSyncStruct extends IThriftStruct {
  public static function fromShape(this::TConstructorShape $shape)[]: this;
}

/**
 * Base interface for Thrift structs with async APIs
 *
 * This is used for structs that directly or indirectly have a field that is
 * wrapped using FieldWrapper annotation.
 *
 * FieldWrapper annotation enables wrapping the field within a wrapper class.
 * Such wrapped field can only be accessed via this class.
 *
 * The wrapper class supports async APIs. Hence, struct APIs that invoke wrapper
 * methods need to be async as well. This interface is used by any struct
 * that itself has a wrapped field or is dependent on a struct with wrapped
 * fields.
 */
<<__ConsistentConstruct, Oncalls('thrift')>> // @oss-disable
// @oss-enable <<__ConsistentConstruct>>
interface IThriftAsyncStruct extends IThriftStruct {
  public static function genFromShape(
    this::TConstructorShape $shape,
  )[zoned_shallow]: Awaitable<this>;
}
