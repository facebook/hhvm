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

/**
 * Immutable Wrapper class on Thrift structs exposing generic capabilities.
 *
 * Hack stores complex types by reference. This means that the underlying
 * thrift object is a reference. To prevent any accidental mutation to the
 * original object, one must only expose through this wrapper readonly methods
 * that return:
 * - primtive types, e.g., bool, int, float, string,
 * - or value types such as Hack arrays https://docs.hhvm.com/hack/arrays-and-collections/mutating-values
 * - or a (deep) copy of a complex type,
 * - or alternatively (not recommended) a nested immutable thrift struct, which
 * is subject to the same rules above.
 *
 */
<<__ConsistentConstruct, Oncalls('signals_infra')>>
abstract class ThriftImmutableWrapper
  implements JsonSerializable, \HH\IMemoizeParam {

  abstract const type TThrift as IThriftShapishSyncStruct;
  const type TShape = this::TThrift::TShape;

  final public function __construct(protected this::TThrift $data) {}

  final public static function createInstance(?this::TThrift $data): ?this {
    return $data is null ? null : new static($data);
  }

  <<__Override>>
  final public function jsonSerialize(): mixed {
    return $this->data->__toShape();
  }

  <<__Override>>
  final public function getInstanceKey(): string {
    return $this->data->getInstanceKey();
  }

  final public function createDeepCopy(): this::TThrift {
    return ThriftCloner::clone($this->data);
  }

  final public function serialize(
    SignalsPipeSerializationProtocol $protocol =
      SignalsPipeSerializationProtocol::JSON,
  ): string {
    return SignalsPipeUtils::serializeThrift($this->data, $protocol);
  }

  public function toString(): string {
    return JSON::encode($this->data);
  }

  public function toShape(): this::TShape {
    return $this->data->__toShape();
  }

}
