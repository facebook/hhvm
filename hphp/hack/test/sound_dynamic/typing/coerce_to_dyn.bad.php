<?hh

class C {}

function d(dynamic $d) : void {}

function test_prim(mixed $m, nonnull $n, C $c) : void {
  //hh_log_level('sub', 2);
  d($c);
  d($m);
  d($n);
}

function test_contain((string, C) $t, vec<C> $v, dict<string, C> $di,
                      darray<arraykey, C> $da,
                      varray<C> $va)
  : void {
  d($t);
  d($v);
  d($di);
  d($da);
  d($va);
}

function test_shape(shape('x' => int, ?'y' => vec<C>) $s) : void {
  d($s);
}


function test_container_context1(vec<dynamic> $v,
                                 dict<arraykey, dynamic> $d,
                                 darray<arraykey, dynamic> $da,
                                 varray<dynamic> $va) : void {

}

function test_container_context2(vec<C> $v,
                                 dict<int, C> $d,
                                 darray<arraykey, C> $da,
                                 varray<C> $va) : void {
  test_container_context1($v, $d, $da, $va);
}

function test_shape_context1(shape('x' => dynamic, ?'y' => dynamic) $s) : void {
}

function test_shape_context2(shape('x' => int, ?'y' => C) $s) : void {
  test_shape_context1($s);
}

class V1<T> {};
class V2<+T> {};

function test_class_context1(V1<dynamic> $v1, V2<dynamic> $v2) : void {}

function test_class_context2(V1<int> $v1, V2<C> $v2) : void {
  test_class_context1($v1, $v2);
}

function test_union(int $i, C $c, bool $b) : void {
  if ($b) {
    $x = $i;
  } else {
    $x = $c;
  }
  d($x);
}
