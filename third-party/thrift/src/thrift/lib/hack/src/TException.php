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
 * Can be called with standard Exception constructor (message, code) or with
 * Thrift Base object constructor (spec, vals).
 *
 * @param mixed $p1 Message (string) or type-spec (array)
 * @param mixed $p2 Code (integer) or values (array)
 */
<<Oncalls('thrift')>> // @oss-disable
class TException extends ExceptionWithPureGetMessage {

  public string $message = '';
  public int $code = 0;

  public function __construct(mixed $p1 = null, mixed $p2 = 0)[] {
    if (HH\is_any_array($p1) && HH\is_any_array($p2)) {
      $spec = $p1;
      $vals = $p2;
      foreach ($spec as $fid => $fspec) {
        $var = HH\FIXME\UNSAFE_CAST<mixed, KeyedContainer<nothing, nothing>>(
          $fspec,
          'FIXME[4063] Revealed by widening type inference of is_array',
        )['var'];
        if (isset($vals[$var])) {
          /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
          $this->$var = $vals[$var];
        }
      }
      parent::__construct();
    } else {
      $error_code = ($p2 ?as int) ?? 0;
      parent::__construct((string)$p1, $error_code);
    }
  }
}
