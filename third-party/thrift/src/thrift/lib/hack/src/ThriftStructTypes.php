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

abstract final class ThriftStructTypes {
  const type TFieldSpec = shape(
    'var' => string,
    'type' => \TType,
    ?'union' => bool,
    ?'etype' => \TType,
    ?'elem' => self::TElemSpec,
    ?'ktype' => \TType,
    ?'vtype' => \TType,
    ?'key' => self::TElemSpec,
    ?'val' => self::TElemSpec,
    ?'format' => string,
    ?'class' => string,
    ?'enum' => string,
    ?'type_annotations' => varray<\IThriftStruct>,
    ?'field_annotations' => varray<\IThriftStruct>,
  );
  const type TElemSpec = shape(
    'type' => \TType,
    ?'etype' => \TType,
    ?'elem' => mixed, // self::TElemSpec once hack supports recursive types
    ?'ktype' => \TType,
    ?'vtype' => \TType,
    ?'key' => mixed, // self::TElemSpec once hack supports recursive types
    ?'val' => mixed, // self::TElemSpec once hack supports recursive types
    ?'format' => string,
    ?'class' => string,
    ?'enum' => string,
  );
  const type TGenericSpec = shape(
    'type' => \TType,
    ?'var' => string,
    ?'union' => bool,
    ?'etype' => \TType,
    ?'elem' => mixed, // self::TElemSpec once hack supports recursive types
    ?'ktype' => \TType,
    ?'vtype' => \TType,
    ?'key' => mixed, // self::TElemSpec once hack supports recursive types
    ?'val' => mixed, // self::TElemSpec once hack supports recursive types
    ?'format' => string,
    ?'class' => classname<\IThriftStruct>,
    ?'enum' => string, // enumname<int>
    ...
  );
}
