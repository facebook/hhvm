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
 * Transport exceptions
 *
 * Types deriving from TTransport exception may not be able to
 * translate their custom error into the set of error code
 * supported by TTransportException. For that the $shortMessage
 * facility is provided.
 *
 * @param mixed  $message Message (string) or type-spec (array)
 * @param mixed  $code Code (integer) or values (array)
 * @param string $shortMessage (string)
 */
<<Oncalls('thrift')>> // @oss-disable
final class TTransportException extends TException {

  const UNKNOWN = 0;
  const NOT_OPEN = 1;
  const ALREADY_OPEN = 2;
  const TIMED_OUT = 3;
  const END_OF_FILE = 4;
  const INVALID_CLIENT = 5;
  const INVALID_FRAME_SIZE = 6;
  const INVALID_TRANSFORM = 7;
  const COULD_NOT_CONNECT = 8;
  const COULD_NOT_READ = 9;
  const COULD_NOT_WRITE = 10;

  protected string $shortMessage;

  public function __construct(
    ?string $message = null,
    int $code = 0,
    string $short_message = '',
  )[] {
    $this->shortMessage = $short_message;
    parent::__construct($message, $code);
  }

  public function getShortMessage()[]: string {
    return $this->shortMessage;
  }

  public function addTAALCategorization(string $datatype): void {
    $datatype_safe = TAAL::stripInvalidCategoryCharacters($datatype);
    $this->message = TAAL::forceCategory($this->message, $datatype_safe);
  }
}
