<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class R<reify T as nonnull> {
  public static function foo(T $x): T {
    return $x;
  }
}

class C {
  public static function foo<reify T as nonnull>(T $x): T {
    return $x;
  }
}

<<__EntryPoint>>
function r(): void {
  R::foo(42); // Bad
  C::foo<int>(42); // Ok
  return;
}
