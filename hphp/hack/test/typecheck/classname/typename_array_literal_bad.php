<?hh

interface I {}
type C1 = I;
newtype C2 = I;
class D {}

function f(string $good1): darray<string, typename<I>> {
  $arr = darray[$good1 => C1::class, 'bad' => D::class, 'good2' => C2::class];
  hh_show($arr);
  return $arr;
}
