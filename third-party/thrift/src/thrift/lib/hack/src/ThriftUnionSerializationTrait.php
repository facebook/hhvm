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
 */

/**
 * Trait for Thrift Unions to call into the Serialization Helper
 */
trait ThriftUnionSerializationTrait implements \IThriftStruct {

  public function read(\TProtocol $protocol): int {
    /* HH_IGNORE_ERROR[4053] every thrift union has a _type field */
    $type = $this->_type;
    $ret = \ThriftSerializationHelper::readUnion($protocol, $this, inout $type);
    /* HH_IGNORE_ERROR[4053] every thrift union has a _type field */
    $this->_type = $type;
    return $ret;
  }

  public function write(\TProtocol $protocol): int {
    // Writing Structs and Unions is the same procedure.
    return \ThriftSerializationHelper::writeStruct($protocol, $this);
  }
}
