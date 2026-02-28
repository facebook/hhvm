<?hh

class C {
  const FOO = 4;
}


<<__EntryPoint>>
function main_remove_key() :mixed{
$s = shape(
  'x' => 4
);

$orig = $s;

$s = Shapes::put($s, 'y', 3);
var_dump($s);
$s = Shapes::put($s, 'x', 'test');
var_dump($s);

var_dump($orig);

$t = shape(
  C::FOO => 5
);

Shapes::put($t, C::FOO, 7);
}
