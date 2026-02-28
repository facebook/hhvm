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
 * Accelerated compact protocol: used in conjunction with a Thrift HPHP
 * extension for faster serialization and deserialization. The generated Thrift
 * code uses instanceof to look for this class and call into the extension.
 */
// @oss-disable: <<Oncalls('thrift')>>
class TCompactProtocolAccelerated extends TCompactProtocolBase {
  // The generated Thrift code calls this as a final check. If it returns true,
  // the HPHP extension will be used; if it returns false, the above PHP code is
  // used as a fallback.
  public static function checkVersion(int $v)[]: bool {
    return $v === 1;
  }

  public function __construct(IThriftBufferedTransport $trans)[] {
    parent::__construct($trans);
  }

  <<__Override>>
  public function writeRPCMessage(
    string $fname,
    TMessageType $type,
    IThriftStruct $message_struct,
    int $seq_id,
    bool $is_one_way = false,
  ): void {
    if (ThriftSerializationHelper::useStructToStringRPCHelpers()) {
      // This should not be reset, but for backwards compatibility
      // using what the client/processor already uses.
      $this->version = TCompactProtocolBase::VERSION;
      parent::writeRPCMessage(
        $fname,
        $type,
        $message_struct,
        $seq_id,
        $is_one_way,
      );
      return;
    }
    thrift_protocol_write_compact2(
      $this,
      $fname,
      $type,
      $message_struct,
      $seq_id,
      $is_one_way,
      // This should be `$this->version`, but for backwards compatibility
      // using what the client/processor already uses.
      TCompactProtocolBase::VERSION,
    );
  }

  <<__Override>>
  public function readRPCMessage<TMessageStruct as IThriftStruct>(
    classname<TMessageStruct> $message_struct_class,
    string $fname,
    ?int $expected_seq_id,
    int $options = 0,
    bool $_compare_seq_id = false,
  ): TMessageStruct {
    if (
      ThriftSerializationHelper::useStructToStringRPCHelpers() &&
      $this->trans_ is TMemoryBuffer
    ) {
      return parent::readRPCMessage(
        $message_struct_class,
        $fname,
        $expected_seq_id,
        $options,
        // This is set to false because hhvm extension doesn't support
        // it yet. So this is mainly for backwards compatibility.
        // Plus the comparison doesn't work when using the same client for concurrent
        // requests regardless of protocol.
        false, // compare_seq_id
      );
    }
    return thrift_protocol_read_compact($this, $message_struct_class, $options);
  }

  <<__Override>>
  public function readRPCStruct<TStruct as IThriftStruct>(
    classname<TStruct> $struct_class,
    int $options = 0,
  ): TStruct {

    if (
      ThriftSerializationHelper::useStructToStringRPCHelpers() &&
      $this->trans_ is TMemoryBuffer
    ) {
      $buffer = $this->trans_->getBuffer();
      // This is necessary for concurrent requests to work with the same client
      // Extension does this at the end by calling putBack()
      // and advancing index pointer as data is read from the buffer.
      $this->trans_->resetBuffer();
      $struct = thrift_protocol_read_compact_struct_from_string(
        $buffer,
        HH\class_to_classname($struct_class),
        $options,
        $this->version,
      );
    } else {
      $struct = thrift_protocol_read_compact_struct(
        $this,
        HH\class_to_classname($struct_class),
        $options,
      );
    }
    $this->readMessageEnd();
    return $struct;
  }

  <<__Override>>
  public function writeRPCStruct(IThriftStruct $struct): void {
    if (ThriftSerializationHelper::useStructToStringRPCHelpers()) {
      $buffer =
        thrift_protocol_write_compact_struct_to_string($struct, $this->version);
      $this->trans_->write($buffer);
      return;
    }
    thrift_protocol_write_compact_struct($this, $struct, $this->version);
  }
}
