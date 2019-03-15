<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class C<T> {
  public static function m(): void {}
}

function f<<<__Newable>> reify T as C<mixed>>(
  T<T> $t // already an error
): T<T> { // already an error
  f<T<T>>($t); // already an error

  new T<T>(); // new error
  T<T>::m(); // new error
  return $t;
}
