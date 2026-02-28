<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class TestVarrayDarrayExpression {
  private static function foo(): void {
    dict['bar' => 0, 'baz' => 1, 'qux' => 2];
    vec[false, true];
  }
}
