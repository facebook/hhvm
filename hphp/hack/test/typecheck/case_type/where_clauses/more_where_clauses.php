<?hh

abstract class Box<T> {
  public T $value;
}
final class IntBox extends Box<int> {
  const type TInt = int; // just for consistent enforcement of $value's type
  public this::TInt $value = 0;
}

case type MyCaseType<T> =
  | int where T as int
  | string where T super int, T super string
  | bool where IntBox as Box<T>;

function test<T>(MyCaseType<T> $x, Box<T> $box): vec<T> {
  if ($x is int) {
    // T <: int
    hh_expect<int>($box->value);
    return vec[0]; // error b/c we don't know int <: T
  } else if ($x is string) {
    // int <: T, string <: T
    return vec[0, "string"];
  } else {
    // IntBox <: Box<T> -> T = int
    hh_expect<Box<int>>($box);
    return vec[0];
  }
}

case type MyCaseType2<T1, T2> =
  | int where T1 super bool, T2 super null
  | string where T1 super int, T1 super string, T2 super float;

function test2<T1, T2>(MyCaseType2<T1, T2> $x, bool $test): (T1, T2) {
  if ($x is int) {
    // bool <: T1, null <: T2
    return tuple(true, null);
  } else {
    // int <: T1, string <: T1, float <: T2
    return $test ? tuple(1, 0.) : tuple("string", 0.);
  }
}
