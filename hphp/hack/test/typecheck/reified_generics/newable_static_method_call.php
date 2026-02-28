<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
abstract class A {
  public abstract static function f(): void;
}

function test_newable_static_method_call<<<__Newable>> reify T as A>(): void {
  $x = new T();
  T::f(); // OK: Since T is newable, it's concrete and has all static methods implemented.
}

function test_non_newable_static_method_call<reify T as A>(): void {
  T::f(); // Error: T is not newable, so calling abstract static method should fail
}
