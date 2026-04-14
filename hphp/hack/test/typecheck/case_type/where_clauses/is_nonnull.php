<?hh

final class MyClass {}

case type NonnullCT<+T> =
  | nonnull where T super MyClass
  | null;

function test_nonnull_refine<T>(NonnullCT<T> $x): T {
  if ($x is (int, nonnull)) {
    return new MyClass();
  }
  throw new Exception();
}

function test_nonnull_refine_negative<T>(NonnullCT<T> $x): T {
  if ($x is (int, nonnull)) {
    return new MyClass();
  } else {
    return new MyClass();  // should error: T not constrained in else branch
  }
}

case type IntOrTuple<+T> =
  | (string, string) where T super MyClass
  | int;

function test_mixed_refine<T>(IntOrTuple<T> $x): T {
  if ($x is (mixed, mixed)) {
    return new MyClass();
  }
  throw new Exception();
}
