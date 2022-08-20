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
 * @package thrift.protocol.binary
 */

/**
 * Utility class for serializing and deserializing
 * a thrift object using TBinaryProtocolAccelerated.
 */
class TBinarySerializer extends TProtocolSerializer {

  // NOTE(rmarin): Because thrift_protocol_write_binary
  // adds a begin message prefix, you cannot specify
  // a transport in which to serialize an object. It has to
  // be a string. Otherwise we will break the compatibility with
  // normal deserialization.
  public static function serialize(
    IThriftStruct $object,
    bool $disable_hphp_extension = false,
  ): string {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);
    if (function_exists('thrift_protocol_write_binary') &&
        !$disable_hphp_extension) {
      thrift_protocol_write_binary(
        $protocol,
        $object->getName(),
        TMessageType::REPLY,
        $object,
        0,
        $protocol->isStrictWrite(),
      );

      $unused_name = null;
      $unused_type = null;
      $unused_seqid = null;
      $protocol->readMessageBegin(
        inout $unused_name,
        inout $unused_type,
        inout $unused_seqid,
      );
    } else {
      $object->write($protocol);
    }
    return $transport->getBuffer();
  }

  public static function deserialize<T as IThriftStruct>(
    string $str,
    T $object,
    bool $disable_hphp_extension = false,
  ): T {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);
    if (function_exists('thrift_protocol_read_binary') &&
        !$disable_hphp_extension) {
      $protocol->writeMessageBegin('', TMessageType::REPLY, 0);
      $transport->write($str);
      $object = thrift_protocol_read_binary(
        $protocol,
        get_class($object),
        $protocol->isStrictRead(),
      );
    } else {
      $transport->write($str);
      $object->read($protocol);
    }
    return $object;
  }
}
