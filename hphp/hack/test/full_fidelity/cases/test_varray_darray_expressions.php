<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class TestVarrayDarrayExpression {
  private static function foo(): void {
    darray['bar' => 0, 'baz' => 1, 'qux' => 2];
    varray[false, true];
  }
}
