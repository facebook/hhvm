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
 * Base class for serializing thrift structs using a TProtocol
 */
<<__ConsistentConstruct, Oncalls('thrift')>> // @oss-disable
abstract class TProtocolSerializer {
  abstract public static function serialize(IThriftStruct $object): string;

  abstract public static function deserialize<T as IThriftStruct>(
    string $str,
    T $object,
  ): T;

  abstract public static function serializeData(
    mixed $object,
    ThriftStructTypes::TGenericSpec $type_spec,
  ): string;

  abstract public static function deserializeData(
    string $str,
    ThriftStructTypes::TGenericSpec $type_spec,
  ): mixed;
}

<<Oncalls('thrift')>> // @oss-disable
abstract class TProtocolWritePropsSerializer extends TProtocolSerializer {
  <<__Override>>
  abstract public static function serialize(
    IThriftStruct $object,
  )[write_props]: string;

  <<__Override>>
  abstract public static function deserialize<T as IThriftStruct>(
    string $str,
    T $object,
  )[write_props]: T;
  <<__Override>>
  abstract public static function serializeData(
    mixed $object,
    ThriftStructTypes::TGenericSpec $type_spec,
  )[write_props]: string;
  <<__Override>>
  abstract public static function deserializeData(
    string $str,
    ThriftStructTypes::TGenericSpec $type_spec,
  )[write_props]: mixed;
}
