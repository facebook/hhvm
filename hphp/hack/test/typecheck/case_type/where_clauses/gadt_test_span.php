<?hh

final class MyInt {}
final class MyBool {}

case type MyCaseType<T> =
  | int where T super MyInt
  | bool where T super MyBool;

function f<T>(MyCaseType<T> $x, nonnull $nn, bool $test): T {
  $y = $test ? $nn : $x;
  if ($y is int) {
    return new MyInt(); // this should error
  } else {
    return new MyBool(); // this should error
  }
}
