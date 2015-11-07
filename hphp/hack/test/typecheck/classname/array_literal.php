<?hh // strict

interface I {}
class C1 implements I {}
class C2 implements I {}
type T1 = I;
newtype T2 = I;
function test(string $bar, string $baz): void {
  $arr = array('foo', 'bar', C1::class, T1::class);
  $index = 0;
  $arr[$index] = 'foo'; // force conversion from tuple-like to vec-like
  hh_show($arr);
  $arr_nested_routing = array(
    'foo/' => array('*' => I::class, $baz => C1::class, 'cdr' => T1::class),
    $bar => C2::class,
    'fiz' => T2::class,
  );
  hh_show($arr_nested_routing);
}
