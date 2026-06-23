<?hh
// `this` inside an *invariant* container is still obtainable: you can read a
// `this` out of it (e.g. Box::get). Invariance pins the element *type*, but the
// runtime contents may be any subtype, so two invariant `Box<this>` can yield
// two values of different runtime classes -> the binary method problem.
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

// `T` is invariant (the default for a class type parameter).
final class Box<T> {}

abstract class A {
  // obtainable=1  -> OK (single occurrence is abstracted into one tparam)
  public static function s_box_one(Box<this> $_): void {}

  // obtainable=2  -> ERROR
  public static function s_box_two(Box<this> $_, Box<this> $_): void {}
  // both `this` are under the invariant Box: obtainable=2 (one param) -> ERROR
  public static function s_box_of_tuple(Box<(this, this)> $_): void {}
  // mixed bare (contravariant) + Box (invariant): obtainable=2 -> ERROR
  public static function s_bare_and_box(this $_, Box<this> $_): void {}

  // instance: receiver(1) + Box(1) = 2 -> ERROR
  public function i_box(Box<this> $_): void {}
}

function test(): void {
  $ok1 = A::s_box_one<>;

  $bad1 = A::s_box_two<>;
  $bad2 = A::s_box_of_tuple<>;
  $bad3 = A::s_bare_and_box<>;
  $bad4 = meth_caller(A::class, 'i_box');
}
