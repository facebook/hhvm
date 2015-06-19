<?hh

$s = shape(
  'x' => 4
);

var_dump(Shapes::keyExists($s, 'x'));
var_dump(Shapes::keyExists($s, 'y'));

class C {
  const FOO = 4;
}

$t = shape(
  C::FOO => 5
);

Shapes::idx($t, C::FOO);
