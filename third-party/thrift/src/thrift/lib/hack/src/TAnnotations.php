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

use namespace FlibSL\{C, Math, Str, Vec}; // @oss-enable

type TStructFieldAnnotations = shape(
  'field' => dict<string, \IThriftStruct>,
  'type' => dict<string, \IThriftStruct>,
);

type TStructAnnotations = shape(
  'struct' => dict<string, \IThriftStruct>,
  'fields' => dict<string, TStructFieldAnnotations>,
);

type TServiceAnnotations = shape(
  'service' => dict<string, \IThriftStruct>,
  'functions' => dict<string, dict<string, \IThriftStruct>>,
);

type TEnumAnnotations = shape(
  'enum' => dict<string, \IThriftStruct>,
  'constants' => dict<string, dict<string, \IThriftStruct>>,
);

// @oss-disable: <<Oncalls('thrift')>>
interface IThriftServiceStaticMetadata {
  public static function getAllStructuredAnnotations(
  )[write_props]: \TServiceAnnotations;
  public static function getServiceMetadata()[]: \tmeta_ThriftService;
  public static function getServiceMetadataResponse(
  )[]: \tmeta_ThriftServiceMetadataResponse;
}

// @oss-disable: <<Oncalls('thrift')>>
interface IThriftEnumStaticMetadata {
  public static function getAllStructuredAnnotations(
  )[write_props]: \TEnumAnnotations;

  public static function getEnumMetadata()[]: \tmeta_ThriftEnum;
}

// @oss-disable: <<Oncalls('thrift')>>
interface IThriftConstants {
  public static function getAllStructuredAnnotations(
  )[write_props]: dict<string, dict<string, \IThriftStruct>>;
}

// @oss-disable: <<Oncalls('thrift')>>
interface IThriftExceptionMetadata {
  public static function getExceptionMetadata()[]: \tmeta_ThriftException;
}

// @oss-disable: <<Oncalls('thrift')>>
interface IThriftStructMetadata extends IThriftStruct {
  public static function getStructMetadata()[]: \tmeta_ThriftStruct;
}
