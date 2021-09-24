<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

class C {}

function test_prim(mixed $m, nonnull $n, C $c) : void {
  //hh_log_level('sub', 2);
  $c upcast dynamic;
  $m upcast dynamic;
  $n upcast dynamic;
}

function test_contain((string, C) $t, vec<C> $v, dict<string, C> $di,
                      darray<arraykey, C> $da,
                      varray<C> $va)
  : void {
  $t upcast dynamic;
  $v upcast dynamic;
  $di upcast dynamic;
  $da upcast dynamic;
  $va upcast dynamic;
}

function test_shape(shape('x' => int, ?'y' => vec<C>) $s) : void {
  $s upcast dynamic;
}


function test_container_context(vec<C> $v,
                                dict<int, C> $d,
                                darray<arraykey, C> $da,
                                varray<C> $va) : void {
  $v upcast vec<dynamic>;
  $d upcast dict<arraykey, dynamic>;
}

function test_shape_context(shape('x' => int, ?'y' => C) $s) : void {
  $s upcast shape('x' => dynamic, ?'y' => dynamic);
}

class V1<<<__RequireDynamic>> T> {};
class V2<<<__RequireDynamic>> +T> {};

function test_class_context(V1<int> $v1, V2<C> $v2) : void {
  $v1 upcast V1<dynamic>;
  $v2 upcast V2<dynamic>;
}

function test_union(int $i, C $c, bool $b) : void {
  if ($b) {
    $x = $i;
  } else {
    $x = $c;
  }
  $x upcast dynamic;
}
