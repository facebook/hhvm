<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class D<T> {
  public static function m(): void {}
}

function f(): void {
  D<string>::m();
}
