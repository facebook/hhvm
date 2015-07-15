<?hh // strict

interface I {}
class C1 implements I {}
class C2 implements I {}

function test(): void {
  // Some unification special cases unify together
  // classnames and strings for perf / expediency
  $arr = array('foo', 'bar', C1::class);
  hh_show($arr);
  $arr_nested_routing = array(
    'foo/' => array('*' => I::class, 'baz' => C1::class),
    'bar' => C2::class,
  );
  hh_show($arr_nested_routing);
}
