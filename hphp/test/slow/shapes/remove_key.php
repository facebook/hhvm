<?hh

class C {
  const FOO = 4;
}


<<__EntryPoint>>
function main_remove_key() :mixed{
$s = shape(
  'x' => 4
);

Shapes::removeKey(inout $s, 'y');
var_dump($s);
Shapes::removeKey(inout $s, 'x');
var_dump($s);

$t = shape(
  C::FOO => 5
);

Shapes::removeKey(inout $t, C::FOO);
}
