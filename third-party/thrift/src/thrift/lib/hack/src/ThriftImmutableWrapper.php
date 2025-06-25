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
    return TCompactSerializer::deserialize(
      TCompactSerializer::serialize($this->data),
      Classnames::getx($this->data)
        |> HH\classname_to_class($$)
        |> $$::withDefaultValues(),
    );
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
