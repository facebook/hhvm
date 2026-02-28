<?hh

function make($a, $b, $c, $d) :mixed{
  return vec[$a, $b, $c, $d];
}
function make_big($a, $b, $c, $d, $e, $f, $g, $h, $i, $j) :mixed{
  return vec[$a, $b, $c, $d, $e, $f, $g, $h, $i, $j];
}

<<TestAttr(vec["testattr1", "testattr2"])>>
class C {
  public static $vec1 = vec[];
  public static $vec2 = vec[1000, 1001, 1002];
  public static $vec3 = vec['C1', 'C2', 'C3'];

  public $vec4 = vec["public1", "public2"];

  const VEC5 = vec["const1", "const2", "const3", "const4"];
}

<<__EntryPoint>> function main(): void {
  $v1 = vec[];
  $v2 = vec[1, 2, 3, 4, 5];
  $v3 = vec['a', 'b', 'c', 'd', 'e'];
  $v4 = make(100, 200, 300, 400);
  $v5 = make('z', 'y', 'x', 'w');
  $v6 = make_big(10, 20, 30, 40, 50, 60, 70, 80, 90, 100);
  $v7 = make_big('f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o');
  $c = new C;
  var_dump($v1, $v2, $v3, $v4, $v5, $v6, $v7,
           C::$vec1, C::$vec2, C::$vec3, $c->vec4,
           $c::VEC5, (new ReflectionClass('C'))->getAttributes());
}
