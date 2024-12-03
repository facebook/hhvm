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

<<Oncalls('thrift')>> // @oss-disable
final class TApplicationException extends TException implements IThriftStruct {

  use ThriftSerializationTrait;

  const ThriftStructTypes::TSpec SPEC = dict[
    1 => shape(
      'var' => 'message',
      'type' => TType::STRING,
    ),
    2 => shape(
      'var' => 'code',
      'type' => TType::I32,
    ),
  ];
  const dict<string, int> FIELDMAP = dict[
    'message' => 1,
    'code' => 2,
  ];
  const type TConstructorShape = shape(
    ?'message' => ?string,
    ?'code' => ?int,
  );

  const int STRUCTURAL_ID = 0;

  const UNKNOWN = 0;
  const UNKNOWN_METHOD = 1;
  const INVALID_MESSAGE_TYPE = 2;
  const WRONG_METHOD_NAME = 3;
  const BAD_SEQUENCE_ID = 4;
  const MISSING_RESULT = 5;
  const INVALID_TRANSFORM = 6;

  public function __construct(?string $message = null, ?int $code = null)[] {
    parent::__construct($message, $code);
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public static function fromShape(self::TConstructorShape $shape)[]: this {
    return
      new static(Shapes::idx($shape, 'message'), Shapes::idx($shape, 'code'));
  }

  public function getName()[]: string {
    return 'TApplicationException';
  }

  public static function getAllStructuredAnnotations()[]: \TStructAnnotations {
    return shape(
      'struct' => dict[],
      'fields' => dict[
        'message' => shape(
          'field' => dict[],
          'type' => dict[],
        ),
        'code' => shape(
          'field' => dict[],
          'type' => dict[],
        ),
      ],
    );
  }

  public function getInstanceKey()[write_props]: string {
    return TCompactSerializer::serialize($this);
  }
}
