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
 * @package thrift
 */

/**
 * Base interface for Thrift structs
 */
<<__ConsistentConstruct>>
interface IThriftStruct extends \HH\IMemoizeParam {
  const type TFieldSpec = shape(
    'var' => string,
    'type' => TType,
    ?'union' => bool,
    ?'etype' => TType,
    ?'elem' => this::TElemSpec,
    ?'ktype' => TType,
    ?'vtype' => TType,
    ?'key' => this::TElemSpec,
    ?'val' => this::TElemSpec,
    ?'format' => string,
    ?'class' => string,
    ?'enum' => string,
    ?'type_annotations' => varray<IThriftStruct>,
    ?'field_annotations' => varray<IThriftStruct>,
  );
  const type TElemSpec = shape(
    'type' => TType,
    ?'etype' => TType,
    ?'elem' => mixed, // this::TElemSpec once hack supports recursive types
    ?'ktype' => TType,
    ?'vtype' => TType,
    ?'key' => mixed, // this::TElemSpec once hack supports recursive types
    ?'val' => mixed, // this::TElemSpec once hack supports recursive types
    ?'format' => string,
    ?'class' => string,
    ?'enum' => string,
  );
  const type TGenericSpec = shape(
    'type' => TType,
    ?'var' => string,
    ?'union' => bool,
    ?'etype' => TType,
    ?'elem' => mixed, // this::TElemSpec once hack supports recursive types
    ?'ktype' => TType,
    ?'vtype' => TType,
    ?'key' => mixed, // this::TElemSpec once hack supports recursive types
    ?'val' => mixed, // this::TElemSpec once hack supports recursive types
    ?'format' => string,
    ?'class' => classname<IThriftStruct>,
    ?'enum' => string, // classname<HH\BuiltinEnum<int>>
    ...
  );

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
