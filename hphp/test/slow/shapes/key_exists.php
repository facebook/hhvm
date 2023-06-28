<?hh

class C {
  const FOO = 4;
}


<<__EntryPoint>>
function main_key_exists() :mixed{
$s = shape(
  'x' => 4
);

var_dump(Shapes::keyExists($s, 'x'));
var_dump(Shapes::keyExists($s, 'y'));

$t = shape(
  C::FOO => 5
);

Shapes::idx($t, C::FOO);
}
