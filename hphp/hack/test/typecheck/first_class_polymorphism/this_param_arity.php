<?hh
// Bare `this` parameters: counting and the >= 2 threshold, plus the implicit
// receiver of instance methods.
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class A {
  // c=1            -> OK (single contravariant `this` is the `id`-like case)
  public static function s_one(this $_): void {}
  // c=2            -> ERROR
  public static function s_two(this $_, this $_): void {}
  // c=2 (one param) -> ERROR
  public static function s_tuple((this, this) $_): void {}
  // c=3            -> ERROR
  public static function s_three(this $_, this $_, this $_): void {}

  // receiver(1)             -> OK
  public function i_none(int $_): void {}
  // receiver(1) + c=1 = 2   -> ERROR
  public function i_one(this $_): void {}
}

function test(): void {
  $ok1 = A::s_one<>;
  $bad1 = A::s_two<>;
  $bad2 = A::s_tuple<>;
  $bad3 = A::s_three<>;

  $ok2 = meth_caller(A::class, 'i_none');
  $bad4 = meth_caller(A::class, 'i_one');
}
