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
 * Note: this base class must only expose readonly methods and must never
 * allow a caller to mutate the underlying thrift object.
 */
<<__ConsistentConstruct, Oncalls('signals_infra')>>
abstract class ThriftImmutableWrapper<T as \IThriftShapishSyncStruct>
  implements JsonSerializable, \HH\IMemoizeParam {
  final public function __construct(protected T $data) {}

  final public static function createInstance(?T $data): ?this {
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

  final public function createDeepCopy(): T {
    return TCompactSerializer::deserialize(
      TCompactSerializer::serialize($this->data),
      Classnames::get($this->data) as nonnull
        |> HH\classname_to_class($$)
        |> $$::withDefaultValues(),
    );
  }

  final public function serialize(
    SignalsPipeSerializationProtocol $protocol =
      SignalsPipeSerializationProtocol::JSON,
  ): string {
    return SignalsPipeUtils::serializeThrift($this->data, $protocol) ?? '';
  }

  public function toString(): string {
    return JSON::encode($this->data);
  }
}
