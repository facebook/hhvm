<?hh

class C {
  const A = 1;

  public varray $p0 = vec[2, 1, 3];
  public varray<int> $p1 = vec[C::A, 1];
  public darray $p2 = dict['a' => 2, 'b' => 1, 'c' => 3];
  public darray<string, mixed> $p3 = dict['a' => C::A, 'b' => 1];
}

function f(
  varray $p0 = vec[C::A, 1],
  varray<int> $p1 = vec[2, 1, 3],
  ?varray $p2 = vec[1, 2, 3],
  varray $p3 = vec[vec[], vec[vec[1, 2, 3], vec[]], vec[1, 2, 3]],
  varray $p4 = vec[vec[], vec[vec[1, C::A, 3], vec[]], vec[1, 2, 3]],
  varray $p5 = vec[vec[], vec[vec[1, 2, 3], vec[]], vec[1, 2, 3]],
  varray $p6 = vec[vec[], vec[vec[1, C::A, 3], vec[]], vec[1, 2, 3]],
  varray $p7 = tuple(1, 2, 3),
  varray $p8 = tuple(C::A, 1, 2)
) :mixed{}

function g(
  darray $p0 = dict[1 => C::A, 2 => 1],
  darray<int, int> $p1 = dict[1 => 2, 4 => 1, 3 => 3],
  ?darray $p2 = dict[1 => 1, "hi" => 2, 5 => 3],
  darray $p3 = dict[1 => dict[], "hi" => vec[dict["key1" => 123, "key2" => dict[]], 456], 5 => dict[]],
  darray $p4 = dict[1 => dict[], "hi" => vec[dict["key1" => C::A, "key2" => dict[]], 456], 5 => dict[]],
  darray $p5 = dict[1 => dict[], "hi" => dict[2 => dict["key1" => 123, "key2" => dict[]], 3 => 456], 5 => dict[]],
  darray $p6 = dict[1 => dict[], "hi" => dict[2 => dict["key1" => C::A, "key2" => dict[]], 3 => 456], 5 => dict[]],
  darray $p7 = shape('a' => 1, 'b' => 2),
  darray $p8 = shape('a' => C::A, 'b' => 2)
) :mixed{}

function test($func) :mixed{
  foreach ((new ReflectionFunction($func))->getParameters() as $p) {
    var_dump($p->getDefaultValueText());
  }

  echo "======================================================\n";

  foreach ((new ReflectionFunction($func))->getParameters() as $p) {
    var_dump($p->getDefaultValue());
  }
  foreach ((new ReflectionFunction($func))->getParameters() as $p) {
    $x = $p->getDefaultValue();
    var_dump(is_varray($x));
    var_dump(is_darray($x));
  }

  echo "======================================================\n";

  foreach ((new ReflectionFunction($func))->getParameters() as $p) {
    var_dump($p->getTypehintText());
  }

  echo "======================================================\n";

  foreach ((new ReflectionFunction($func))->getParameters() as $p) {
    var_dump($p->getTypeText());
  }

  echo "======================================================\n";
}


<<__EntryPoint>>
function main_reflection() :mixed{
test('f');
test('g');

$rp = new ReflectionProperty('C', 'p0');
var_dump($rp->getDefaultValue());
var_dump(is_varray($rp->getDefaultValue()));
var_dump($rp->getTypeText());

$rp = new ReflectionProperty('C', 'p1');
var_dump($rp->getDefaultValue());
var_dump(is_varray($rp->getDefaultValue()));
var_dump($rp->getTypeText());

$rp = new ReflectionProperty('C', 'p2');
var_dump($rp->getDefaultValue());
var_dump(is_darray($rp->getDefaultValue()));
var_dump($rp->getTypeText());

$rp = new ReflectionProperty('C', 'p3');
var_dump($rp->getDefaultValue());
var_dump(is_darray($rp->getDefaultValue()));
var_dump($rp->getTypeText());
}
