<?hh

class C {
  const FOO = 4;
}


<<__EntryPoint>>
function main_remove_key() {
$s = shape(
  'x' => 4
);

Shapes::removeKey(&$s, 'y');
var_dump($s);
Shapes::removeKey(&$s, 'x');
var_dump($s);

$t = shape(
  C::FOO => 5
);

Shapes::removeKey(&$t, C::FOO);
}
