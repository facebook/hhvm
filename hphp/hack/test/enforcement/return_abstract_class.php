<?hh

// NOTE: This is not fully safe because we do not have enforcement-preserving
// overrides. A subclass could override get() and return a value that passes
// the subclass's return type check but violates int at runtime. In practice
// this is rare because Hack's type system prevents such overrides, but
// UNSAFE_CAST or other escape hatches in the subclass could break this.
abstract class A {
  abstract public function get(): int;
}

function test(A $a): int {
  return $a->get();
//       ^ enforcement-at-caret
}
