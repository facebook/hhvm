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
 * Accelerated compact protocol: used in conjunction with a Thrift HPHP
 * extension for faster serialization and deserialization. The generated Thrift
 * code uses instanceof to look for this class and call into the extension.
 */
class TCompactProtocolAccelerated extends TCompactProtocolBase {
  // The generated Thrift code calls this as a final check. If it returns true,
  // the HPHP extension will be used; if it returns false, the above PHP code is
  // used as a fallback.
  public static function checkVersion($v) {
    return $v == 1;
  }

  public function __construct($trans) {
    // If the transport doesn't implement putBack, wrap it in a
    // TBufferedTransport (which does)
    if (!($trans instanceof IThriftBufferedTransport)) {
      $trans = new TBufferedTransport($trans);
    }

    parent::__construct($trans);
  }
}
