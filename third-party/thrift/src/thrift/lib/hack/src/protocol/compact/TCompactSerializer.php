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
 * @package thrift.protocol.compact
 */

/**
 * Utility class for serializing and deserializing
 * a thrift object using TCompactProtocol.
 */
class TCompactSerializer extends TProtocolSerializer {

  public static function serialize(
    IThriftStruct $object,
    ?int $override_version = null,
    bool $disable_hphp_extension = false,
  ): string {
    $transport = new TMemoryBuffer();
    $protocol = new TCompactProtocolAccelerated($transport);

    $use_hphp_extension =
      function_exists('thrift_protocol_write_compact') &&
      !$disable_hphp_extension;

    $last_version = null;
    if ($override_version !== null) {
      $protocol->setWriteVersion($override_version);
      if (function_exists('thrift_protocol_set_compact_version')) {
        $last_version =
          thrift_protocol_set_compact_version($override_version);
      } else {
        $use_hphp_extension = false;
      }
    }

    if ($use_hphp_extension) {
      thrift_protocol_write_compact(
        $protocol,
        $object->getName(),
        TMessageType::REPLY,
        $object,
        0,
      );
      if ($last_version !== null) {
        thrift_protocol_set_compact_version($last_version);
      }
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
    ?int $override_version = null,
    bool $disable_hphp_extension = false,
  ) {
    $transport = new TMemoryBuffer();
    $protocol = new TCompactProtocolAccelerated($transport);

    $use_hphp_extension =
      function_exists('thrift_protocol_read_compact') &&
      !$disable_hphp_extension;

    if ($override_version !== null) {
      $protocol->setWriteVersion($override_version);
      if (!function_exists('thrift_protocol_set_compact_version')) {
        $use_hphp_extension = false;
      }
    }

    if ($use_hphp_extension) {
      $protocol->writeMessageBegin('', TMessageType::REPLY, 0);
      $transport->write($str);
      $object = thrift_protocol_read_compact($protocol, get_class($object));
    } else {
      $transport->write($str);
      /* HH_FIXME[2060] Trust me, I know what I'm doing */
      $object->read($protocol);
    }
    return $object;
  }
}
