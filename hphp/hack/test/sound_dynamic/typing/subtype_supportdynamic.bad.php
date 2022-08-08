<?hh

function e(supportdyn<nonnull> $sd): void {}

class C {}

function test_prim(mixed $m, nonnull $nn, C $c): void {
  e($m);
  e($nn);
  e($c);
}

function test_contain((string, C) $t, vec<C> $v, dict<string, C> $di,
                      darray<arraykey, C> $da,
                      varray<C> $va): void {
  e($t);
  e($v);
  e($di);
  e($da);
  e($va);
}

function test_shape(shape('x' => int, ?'y' => vec<C>) $s): void {
  e($s);
}


function test_container_context(vec<C> $v,
                                dict<int, C> $d,
                                darray<arraykey, C> $da,
                                varray<C> $va): void {
  e($v);
  e($d);
}

function test_shape_context(shape('x' => int, ?'y' => C) $s): void {
  e($s);
}

class V1<<<__RequireDynamic>> T> {};
class V2<<<__RequireDynamic>> +T> {};

function test_class_context(V1<int> $v1, V2<C> $v2): void {
  e($v1);
  e($v2);
}

function test_union(int $i, C $c, bool $b): void {
  if ($b) {
    $x = $i;
  } else {
    $x = $c;
  }
  e($x);
}

function test_null_things(null $n, ?int $ni): void {
  e($n);
  e($ni);
}
