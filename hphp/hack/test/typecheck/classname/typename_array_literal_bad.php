<?hh // strict

interface I {}
type C1 = I;
newtype C2 = I;
class D {}

function f(string $good1): array<string, typename<I>> {
  $arr = array($good1 => C1::class, 'bad' => D::class, 'good2' => C2::class);
  hh_show($arr);
  return $arr;
}
