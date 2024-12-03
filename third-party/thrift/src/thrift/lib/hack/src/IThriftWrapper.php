<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
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

<<
  __Sealed(
    IThriftFieldWrapper::class,
    IThriftTypeWrapper::class,
    IThriftStructWrapper::class,
  ),
  Oncalls('thrift'),
>> // @oss-disable
abstract class IThriftWrapper<TThriftType> {
  protected function __construct(protected TThriftType $value)[] {}

  final public function getValue_DO_NOT_USE_THRIFT_INTERNAL()[]: TThriftType {
    return $this->value;
  }

  final public function setValue_DO_NOT_USE_THRIFT_INTERNAL(
    TThriftType $value,
  )[write_props]: void {
    $this->value = $value;
  }

  final public static function toThrift_DO_NOT_USE_THRIFT_INTERNAL(
    this $wrapped_value,
  )[zoned_shallow]: TThriftType {
    return $wrapped_value->value;
  }

  abstract public static function genToThrift(
    this $wrapped_value,
  )[zoned_shallow]: Awaitable<TThriftType>;

  abstract public function genUnwrap()[zoned_shallow]: Awaitable<TThriftType>;
  abstract public function genWrap(
    TThriftType $value,
  )[zoned_local]: Awaitable<void>;
}

<<__ConsistentConstruct, Oncalls('thrift')>> // @oss-disable
// @oss-enable << __ConsistentConstruct>>
abstract class IThriftFieldWrapper<
  TThriftType,
  TThriftStruct as IThriftAsyncStruct,
> extends IThriftWrapper<TThriftType> {
  protected function __construct(
    TThriftType $value,
    protected int $fieldId,
    protected TThriftStruct $struct,
  )[] {
    parent::__construct($value);
  }

  final public static function fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
    <<__Explicit>> TThriftType__,
    <<__Explicit>> TThriftStruct__ as IThriftAsyncStruct,
  >(TThriftType__ $value, int $field_id, TThriftStruct__ $parent)[]: this {
    return new static($value, $field_id, $parent);
  }

  abstract public static function genFromThrift<
    <<__Explicit>> TThriftType__,
    <<__Explicit>> TThriftStruct__ as IThriftAsyncStruct,
  >(TThriftType__ $value, int $field_id, TThriftStruct__ $parent)[
    zoned_shallow,
  ]: Awaitable<IThriftFieldWrapper<TThriftType__, TThriftStruct__>>;
}

<<__ConsistentConstruct, Oncalls('thrift')>> // @oss-disable
// @oss-enable << __ConsistentConstruct>>
abstract class IThriftTypeWrapper<TThriftType>
  extends IThriftWrapper<TThriftType> {
  protected function __construct(TThriftType $value)[] {
    parent::__construct($value);
  }

  final public static function fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
    <<__Explicit>> TThriftType__,
  >(TThriftType__ $value)[]: this {
    return new static($value);
  }

  abstract public static function genFromThrift<<<__Explicit>> TThriftType__>(
    TThriftType__ $value,
  )[zoned_shallow]: Awaitable<IThriftTypeWrapper<TThriftType__>>;
}

<<__ConsistentConstruct, Oncalls('thrift')>> // @oss-disable
// @oss-enable << __ConsistentConstruct>>
abstract class IThriftStructWrapper<TThriftStructType as ?IThriftStruct>
  extends IThriftWrapper<TThriftStructType> {
  protected function __construct(TThriftStructType $value)[] {
    parent::__construct($value);
  }

  final public static function fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
    <<__Explicit>> TThriftType__ as ?IThriftStruct,
  >(TThriftType__ $value)[]: this {
    return new static($value);
  }

  abstract public static function genFromThrift<
    <<__Explicit>> TThriftType__ as ?IThriftStruct,
  >(
    TThriftType__ $value,
  )[zoned_shallow]: Awaitable<IThriftStructWrapper<TThriftType__>>;
}
