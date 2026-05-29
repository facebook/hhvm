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

/**
 * Trait for Thrift Structs to call into the Serialization Helper
 */
interface IThriftWillReadHook {
  public function willReadThriftHook(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: void;
}

interface IThriftDidReadHook {
  public function didReadThriftHook(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: void;
}

interface IThriftWillWriteHook {
  public function willWriteThriftHook(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: void;
}

interface IThriftDidWriteHook {
  public function didWriteThriftHook(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: void;
}

// @oss-disable: <<Oncalls('thrift')>>
trait ThriftSerializationTrait implements IThriftStruct {

  protected function willReadThrift(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: void {
    if ($this is IThriftWillReadHook) {
      $this->willReadThriftHook($protocol);
    }
  }

  protected function didReadThrift(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: void {
    if ($this is IThriftDidReadHook) {
      $this->didReadThriftHook($protocol);
    }
  }

  protected function willWriteThrift(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: void {
    if ($this is IThriftWillWriteHook) {
      $this->willWriteThriftHook($protocol);
    }
  }

  protected function didWriteThrift(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: void {
    if ($this is IThriftDidWriteHook) {
      $this->didWriteThriftHook($protocol);
    }
  }

  public function read(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: int {
    $this->willReadThrift($protocol);
    if ($this is IThriftStructWithClearTerseFields) {
      $this->clearTerseFields();
    }
    $result = ThriftSerializationHelper::readStruct($protocol, $this);
    $this->didReadThrift($protocol);
    return $result;
  }

  public function write(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: int {
    $this->willWriteThrift($protocol);
    $result = ThriftSerializationHelper::writeStruct($protocol, $this);
    $this->didWriteThrift($protocol);
    return $result;
  }
}
