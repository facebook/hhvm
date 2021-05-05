<?hh

<<__SupportDynamicType>>
class C {
  public function m() : void {}
}

function d(dynamic $d) : void {}

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
  d($e);
}

function test_prim(bool $b, int $i, float $f, num $n, string $s, arraykey $ak)
  : void {
  d($b);
  d($i);
  d($f);
  d($n);
  d($s);
  $c = new C() as dynamic;
  d($c->m());
  d(null);
  d($ak);
  d(E1::CONST1);
  d(E2::CONST2);
  d(E3::CONST3);
  d(E4::CONST4);
}

function test_contain((string, int) $t, vec<int> $v, dict<string, int> $di,
                      keyset<arraykey> $ks, darray<arraykey, vec<C>> $da,
                      varray<int> $va, ?int $oi)
  : void {
  d($t);
  d($v);
  d($di);
  d($ks);
  d($da);
  d($va);
  d($oi);
}

function test_shape(shape('x' => int, ?'y' => vec<C>) $s) : void {
  d($s);
}

<<__SupportDynamicType>>
class D<<<__NoRequireDynamic>> T> {}

class E {}

function test_class(C $c, D<int> $di, D<E> $de) : void {
  d($c);
  d($di);
  d($de);
}

function test_container_context1(vec<dynamic> $v,
                                 dict<arraykey, dynamic> $d,
                                 darray<arraykey, dynamic> $da,
                                 varray<dynamic> $va,
                                 ?dynamic $o) : void {

}

function test_container_context2(vec<int> $v,
                                 dict<int, C> $d,
                                 darray<arraykey, int> $da,
                                 varray<int> $va,
                                 ?int $o) : void {
  test_container_context1($v, $d, $da, $va, $o);
}

function test_shape_context1(shape('x' => dynamic, ?'y' => dynamic) $s) : void {
}

function test_shape_context2(shape('x' => int, ?'y' => vec<C>) $s) : void {
  test_shape_context1($s);
}

class V2<+T> {};
class V3<-T> {};

function test_class_context1(V2<dynamic> $v2, V3<dynamic> $v3) : void {}

function test_class_context2(V2<int> $v2, V3<mixed> $v3) : void {
  test_class_context1($v2, $v3);
}

function test_union(int $i, C $c, bool $b) : void {
  if ($b) {
    $x = $i;
  } else {
    $x = $c;
  }
  d($x);
}

function test_inter(int $i, bool $b) : void {
  if ($i is E) {
    d($i);
  }
}
