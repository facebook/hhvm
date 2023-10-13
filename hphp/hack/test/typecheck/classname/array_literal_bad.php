<?hh

interface I {}
class C1 implements I {}
class C2 implements I {}
class D {}

function f(string $good1): darray<string, classname<I>> {
  $arr = darray[$good1 => C1::class, 'bad' => D::class, 'good2' => C2::class];
  hh_show($arr);
  return $arr;
}
