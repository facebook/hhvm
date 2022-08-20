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
 * @package thrift.protocol.simplejson
 */

/**
 * Utility class for serializing
 * a thrift object using TSimpleJSONProtocol
 */
class TSimpleJSONSerializer extends TProtocolSerializer {
  public static function serialize(IThriftStruct $object): string {
    $transport = new TMemoryBuffer();
    $protocol = new TSimpleJSONProtocol($transport);
    $object->write($protocol);
    return $transport->getBuffer();
  }

  public static function deserialize<T as IThriftStruct>(
    string $str,
    T $object,
  ): T {
    $transport = new TMemoryBuffer($str);
    $protocol = new TSimpleJSONProtocol($transport);
    $object->read($protocol);
    return $object;
  }
}
