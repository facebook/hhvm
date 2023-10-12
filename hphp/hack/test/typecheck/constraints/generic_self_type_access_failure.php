<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// This code attempts to access T::T and should fail.
interface Intf {};
abstract class Box {
  abstract const type T as Intf;
}

// class provides constrained function
abstract class AbstractGeneric<TX as Box> {
  abstract public function f<T>(T $t): void
  where TX::T = T;
}

class Consumer<TX as Box> {
  // incorrectly constrains AbstractGeneric class
  public function crash<T as Intf>(AbstractGeneric<T> $arg, T $t): void
  where TX::T = T {
    $arg->f($t);
  }
}
