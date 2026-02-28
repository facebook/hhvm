<?hh

final class MyInt {}
final class MyString {}
final class MyBool {}

case type LiftableTo<+T> =
  | int where T super MyInt
  | string where T super MyString
  | bool where T super MyBool;

function lift<T>(LiftableTo<T> $liftable): T {
  if ($liftable is int) {
    return new MyInt();
  } else if ($liftable is string) {
    return new MyString();
  } else {
    return new MyBool();
  }
}

function lift_test_overmatch<T>(LiftableTo<T> $liftable): T {
  if ($liftable is num) {
    return new MyInt();
  }
  throw new Exception();
}

function lift_test_ambiguous_match<T>(LiftableTo<T> $liftable): T {
  if ($liftable is arraykey) {
    return new MyInt(); // this should error
  }
  throw new Exception();
}

case type NonnullCT<T> = nonnull where T super MyInt | null;

function test_span<T>(NonnullCT<T> $x): T {
  if ($x is int) {
    return new MyInt();
  } else {
    return new MyInt(); // this should error
  }
}
