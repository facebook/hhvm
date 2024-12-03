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

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

/**
 * Utility class for serializing and deserializing
 * a thrift object using TBinaryProtocolAccelerated.
 */
<<Oncalls('thrift')>> // @oss-disable
final class TBinarySerializer extends TProtocolSerializer {

  // NOTE(rmarin): Because thrift_protocol_write_binary
  // adds a begin message prefix, you cannot specify
  // a transport in which to serialize an object. It has to
  // be a string. Otherwise we will break the compatibility with
  // normal deserialization.
  <<__Override>>
  public static function serialize(
    IThriftStruct $object,
    bool $disable_hphp_extension = false,
  )[write_props, read_globals]: string {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);
    if (!$disable_hphp_extension) {
      HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
        ()[defaults] ==> thrift_protocol_write_binary(
          $protocol,
          $object->getName(),
          TMessageType::REPLY,
          $object,
          0,
          $protocol->isStrictWrite(),
        ),
        'Binary with memory buffer is write_props, but Hack doesn\'t have a '.
        'way to express this atm.',
      );

      $_name = '';
      $_type = -1;
      $_seqid = -1;
      HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
        ()[defaults] ==> $protocol->readMessageBegin(
          inout $_name,
          inout $_type,
          inout $_seqid,
        ),
        'Binary with memory buffer is write_props, but Hack doesn\'t have a '.
        'way to express this atm.',
      );
    } else {
      HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
        ()[defaults] ==> $object->write($protocol),
        'Binary with memory buffer is write_props, but Hack doesn\'t have a '.
        'way to express this atm.',
      );
    }
    return $transport->getBuffer();
  }

  <<__Override>>
  public static function deserialize<T as IThriftStruct>(
    string $str,
    T $object,
    bool $disable_hphp_extension = false,
    bool $should_leave_extra = false,
    int $options = 0,
  )[read_globals, write_props]: T {
    $transport = new TMemoryBuffer();
    $protocol = (new TBinaryProtocolAccelerated($transport))
      ->setOptions($options);
    if (!$disable_hphp_extension) {
      HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
        ()[defaults] ==>
          $protocol->writeMessageBegin('', TMessageType::REPLY, 0),
        'Binary with memory buffer is write_props, but Hack doesn\'t have a '.
        'way to express this atm.',
      );
      $transport->write($str);
      $object = HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
        ()[defaults] ==> thrift_protocol_read_binary(
          $protocol,
          get_class($object),
          $protocol->isStrictRead(),
          $protocol->getOptions(),
        ),
        'Binary with memory buffer is write_props, but Hack doesn\'t have a '.
        'way to express this atm.',
      );
    } else {
      $transport->write($str);
      HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
        ()[defaults] ==> $object->read($protocol),
        'Binary with memory buffer is write_props, but Hack doesn\'t have a '.
        'way to express this atm.',
      );
    }
    return $object;
  }/* BEGIN_STRIP */

  public static function deserialize_DEPRECATED<T as IThriftStruct>(
    mixed $string_object,
    classname<T> $class_name,
    bool $disable_hphp_extension = false,
  )[read_globals, write_props]: T {
    return static::deserialize(
      (string)$string_object,
      HH\classname_to_class($class_name) |> new $$(),
      $disable_hphp_extension,
    );
  }
  /* END_STRIP */

  <<__Override>>
  public static function serializeData(
    mixed $object,
    ThriftStructTypes::TGenericSpec $type_spec,
  )[read_globals, write_props]: string {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);
    HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> ThriftSerializationHelper::writeStructHelper(
        $protocol,
        $type_spec['type'],
        $object,
        $type_spec,
      ),
      '[T133628451] Memory buffer transport would have write_props, but Hack doesn\'t '.
      'have a way to express this atm.',
    );
    return $transport->getBuffer();
  }

  <<__Override>>
  public static function deserializeData(
    string $str,
    ThriftStructTypes::TGenericSpec $type_spec,
  )[read_globals, write_props]: mixed {
    $transport = new TMemoryBuffer();
    $transport->write($str);
    $protocol = new TBinaryProtocolAccelerated($transport);
    return HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> {
        $result = null;
        $has_wrapper = false;
        ThriftSerializationHelper::readStructHelper(
          $protocol,
          $type_spec['type'],
          inout $result,
          $type_spec,
          inout $has_wrapper,
        );
        return $result;
      },
      '[T133628451] Binary with memory buffer is write_props, but Hack doesn\'t have a '.
      'way to express this atm.',
    );
  }

}
