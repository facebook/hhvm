<?hh

function expectNonNull(nonnull $nn):void { }

function e(supportdyn<nonnull> $sd): void {
  expectNonNull($sd);
}

<<__SupportDynamicType>>
class C {}

enum E1: int {
  CONST1 = 1;
}

enum E2: int as int {
  CONST2 = 2;
}

enum E3: string {
  CONST3 = "a";
}

enum E4: string as string {
  CONST4 = "b";
}

function test_enum_type(E4 $e) : void {
  e($e);
}

function test_prim(bool $b, int $i, float $f, num $n, string $s, arraykey $ak)
  : void {
  e($b);
  e($i);
  e($f);
  e($n);
  e($s);
  e(new C());
  e($ak);
  e(E1::CONST1);
  e(E2::CONST2);
  e(E3::CONST3);
  e(E4::CONST4);
}

function test_contain((string, int) $t, vec<int> $v, dict<string, int> $di,
                      keyset<arraykey> $ks, darray<arraykey, vec<C>> $da,
                      varray<int> $va): void {
  e($t);
  e($v);
  e($di);
  e($ks);
  e($da);
  e($va);
}

function test_shape(shape('x' => int, ?'y' => vec<C>) $s): void {
  e($s);
}

<<__SupportDynamicType>>
class D<T> {}

<<__SupportDynamicType>>
class E {}

function test_class(C $c, D<int> $di, D<E> $de): void {
  e($c);
  e($di);
  e($de);
}

function test_container_context(vec<int> $v,
                                dict<int, C> $d): void {
  e($v);
  e($d);
}

function test_shape_context(shape('x' => int, ?'y' => vec<C>) $s): void {
  e($s);
}

function test_union(int $i, C $c, bool $b): void {
  if ($b) {
    $x = $i;
  } else {
    $x = $c;
  }
  e($x);
}

function test_inter(int $i, bool $b): void {
  if ($i is E) {
    e($i);
  }
}

<<__SupportDynamicType>>
function foo(int $x):bool {
  return false;
}
function test_function((function(~int):bool) $f): void {
  e($f);
  e(fun('foo'));
}
