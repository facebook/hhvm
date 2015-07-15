<?hh // strict

interface I {}
class C1 implements I {}
class C2 implements I {}
class D {}

function condition(): bool {
  // UNSAFE_BLOCK
}

function test_ordering_limitation(): array<string, classname<I>> {
  /* unification of array literal types is not entirely sound ...
   * there's an attempt to do piece-wise unification */
  $arr = array('good1' => C1::class, 'bad' => D::class, 'good2' => C2::class);
  hh_show($arr);
  return $arr;
}

function test_ok_if_last(): array<string, classname<I>> {
  $arr = array('good1' => C1::class, 'good2' => C2::class, 'bad' => D::class);
  hh_show($arr);
  return $arr;
}
