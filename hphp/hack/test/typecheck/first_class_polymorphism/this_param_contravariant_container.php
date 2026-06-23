<?hh
// `this` inside a *contravariant* container is covariant overall when that
// container sits in a parameter: you can only *put* a `this` in, never obtain
// one, so it is safe (the double sign-flip cancels out).
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

// `Contra<-T>` is contravariant in T.
final class Contra<-T> {}

abstract class A {
  // contra . contravariant-container = covariant; obtainable=0 -> OK even doubled
  public static function s_contra_two(
    Contra<this> $_,
    Contra<this> $_,
  ): void {}

  // instance: receiver(1) + obtainable(0) = 1 -> OK
  public function i_contra(Contra<this> $_): void {}
}

function test(): void {
  $ok1 = A::s_contra_two<>;
  $ok2 = meth_caller(A::class, 'i_contra');
}
