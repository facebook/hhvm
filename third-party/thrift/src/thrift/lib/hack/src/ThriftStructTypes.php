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

type ThriftStructGenericSpecRecursiveImpl<
  T /* as ThriftStructGenericSpecRecursiveImpl<T> */,
> = shape(
  'type' => TType,
  ?'var' => string,
  ?'union' => bool,
  ?'etype' => TType,
  ?'elem' => T,
  ?'ktype' => TType,
  ?'vtype' => TType,
  ?'key' => T,
  ?'val' => T,
  ?'format' => string,
  ?'class' => class<IThriftStruct>,
  ?'enum' => string, // enumname<int>
  ?'adapter' => class<IThriftAdapter>,
  ?'is_wrapped' => bool,
  ?'is_terse' => bool,
  ?'is_type_wrapped' => bool,
  ?'is_binary' => bool,
  ...
);

type ThriftStructGenericSpecImpl = ThriftStructGenericSpecRecursiveImpl<
  ThriftStructGenericSpecRecursiveImpl<
    ThriftStructGenericSpecRecursiveImpl<
      ThriftStructGenericSpecRecursiveImpl<
        ThriftStructGenericSpecRecursiveImpl<HH_FIXME\MISSING_GENERIC>,
      >,
    >,
  >,
>;

type ThriftStructElemSpecRecursiveImpl<
  T /* as ThriftStructElemSpecRecursiveImpl<T> */,
> = shape(
  'type' => TType,
  ?'etype' => TType,
  ?'elem' => T,
  ?'ktype' => TType,
  ?'vtype' => TType,
  ?'key' => T,
  ?'val' => T,
  ?'format' => string,
  ?'class' => class<IThriftStruct>,
  ?'enum' => string, // enumname<int>
  ?'adapter' => class<IThriftAdapter>,
  ?'is_wrapped' => bool,
  ?'is_terse' => bool,
  ?'is_type_wrapped' => bool,
  ?'is_binary' => bool,
);

type ThriftStructElemSpecImpl = ThriftStructElemSpecRecursiveImpl<
  ThriftStructElemSpecRecursiveImpl<
    ThriftStructElemSpecRecursiveImpl<
      ThriftStructElemSpecRecursiveImpl<HH_FIXME\MISSING_GENERIC>,
    >,
  >,
>;

type ThriftStructFieldSpecImpl = shape(
  'var' => string,
  'type' => TType,
  ?'union' => bool,
  ?'etype' => TType,
  ?'elem' => ThriftStructElemSpecImpl,
  ?'ktype' => TType,
  ?'vtype' => TType,
  ?'key' => ThriftStructElemSpecImpl,
  ?'val' => ThriftStructElemSpecImpl,
  ?'format' => string,
  ?'class' => class<IThriftStruct>,
  ?'enum' => string, // enumname<int>
  ?'adapter' => class<IThriftAdapter>,
  ?'is_wrapped' => bool,
  ?'is_terse' => bool,
  ?'is_type_wrapped' => bool,
  ?'is_binary' => bool,
);

abstract final class ThriftStructTypes {
  const type TFieldSpec = ThriftStructFieldSpecImpl;
  const type TElemSpec = ThriftStructElemSpecImpl;
  // Union of TFieldSpec and TElemSpec. For use in places that operate on both.
  const type TGenericSpec = ThriftStructGenericSpecImpl;
  const type TSpec = dict<int, self::TFieldSpec>;
}
