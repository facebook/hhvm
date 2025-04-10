<?hh

final class MyArraykey {}
final class MyBool {}

case type MyCaseType<T> =
  | arraykey where T super MyArraykey
  | bool where T super MyBool;

function f<T>(MyCaseType<T> $x): T {
  if ($x is int) {
    return new MyArraykey();
  } else {
    return new MyBool(); // this should error
  }
}
