<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class C {
  public function m() : void {}
}

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
  $e upcast dynamic;
}

function test_prim(bool $b, int $i, float $f, num $n, string $s, arraykey $ak)
  : void {
  $b upcast dynamic;
  $i upcast dynamic;
  $f upcast dynamic;
  $n upcast dynamic;
  $s upcast dynamic;
  $c = new C() upcast dynamic;
  $c->m() upcast dynamic;
  null upcast dynamic;
  $ak upcast dynamic;
  E1::CONST1 upcast dynamic;
  E2::CONST2 upcast dynamic;
  E3::CONST3 upcast dynamic;
  E4::CONST4 upcast dynamic;
}

function test_contain((string, int) $t, vec<int> $v, dict<string, int> $di,
                      keyset<arraykey> $ks, darray<arraykey, vec<C>> $da,
                      varray<int> $va, ?int $oi)
  : void {
  $t upcast dynamic;
  $v upcast dynamic;
  $di upcast dynamic;
  $ks upcast dynamic;
  $da upcast dynamic;
  $va upcast dynamic;
  $oi upcast dynamic;
}

function test_shape(shape('x' => int, ?'y' => vec<C>) $s) : void {
  $s upcast dynamic;
}

<<__SupportDynamicType>>
class D<<<__NoRequireDynamic>> T> {}

class E {}

function test_class(C $c, D<int> $di, D<E> $de) : void {
  $c upcast dynamic;
  $di upcast dynamic;
  $de upcast dynamic;
}

function test_container_context(vec<int> $v,
                                dict<int, C> $d,
                                ?int $o) : void {
  $v upcast vec<dynamic>;
  $d upcast dict<arraykey, dynamic>;
  $o upcast ?dynamic;
}

function test_shape_context(shape('x' => int, ?'y' => vec<C>) $s) : void {
  $s upcast shape('x' => dynamic, ?'y' => dynamic);
}

class V2<+T> {};
class V3<-T> {};

function test_class_context(V2<int> $v2, V3<mixed> $v3) : void {
  $v2 upcast V2<dynamic>;
  $v3 upcast V3<dynamic>;
}

function test_union(int $i, C $c, bool $b) : void {
  if ($b) {
    $x = $i;
  } else {
    $x = $c;
  }
  $x upcast dynamic;
}

function test_inter(int $i, bool $b) : void {
  if ($i is E) {
    $i upcast dynamic;
  }
}

function test_expression_helper() : int {
  return 10;
}

function test_expression(int $i) : void {
  $x = test_expression_helper() upcast dynamic;
  $y = ($i + 1) upcast dynamic;
  $z = 1 upcast dynamic;
  $m = (1 upcast int) + 1;
  $n = 1 + (1 upcast int);
  $o = 1 upcast int + 1;
}
