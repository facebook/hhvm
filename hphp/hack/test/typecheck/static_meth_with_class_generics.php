<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C1<reify T as nonnull> {
  public static function foo(T $x): T {
    return $x;
  }
}

class C2 {
  public static function foo<reify T as nonnull>(T $x): T {
    return $x;
  }
}

class C3<T> {
  public static function foo(T $x): T {
    return $x;
  }
}

<<__EntryPoint>>
function r(): void {
  C1::foo(42); // Bad
  C2::foo<int>(42); // Ok
  C3::foo(42); // Ok
  return;
}
