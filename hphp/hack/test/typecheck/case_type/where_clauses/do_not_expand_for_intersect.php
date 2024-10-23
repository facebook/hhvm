<?hh

final class MyInt {}
final class MyString {}
final class MyBool {}

case type LiftableTo<+T> =
  | int where T super MyInt
  | string where T super MyString
  | bool where T super MyBool;

function test<T>(LiftableTo<T> $x): LiftableTo<T> {
  if ($x is int) {
  }
  return $x;
}
