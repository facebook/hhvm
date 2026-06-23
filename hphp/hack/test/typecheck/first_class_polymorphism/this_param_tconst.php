<?hh
// `this::T` type constant projections are never counted (they are elaborated
// into type parameters by extraction), at any nesting depth.
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class A {
  abstract const type T as arraykey;

  // obtainable=0 -> OK
  public static function s_tconst_two(this::T $_, this::T $_): void {}
  public static function s_tconst_nested(
    ?this::T $_,
    vec<this::T> $_,
    shape('x' => this::T) $_,
  ): void {}

  // instance receiver(1) + tconst(0) = 1 -> OK
  public function i_tconst(this::T $_, this::T $_): void {}
}

function test(): void {
  $ok1 = A::s_tconst_two<>;
  $ok2 = A::s_tconst_nested<>;
  $ok3 = meth_caller(A::class, 'i_tconst');
}
