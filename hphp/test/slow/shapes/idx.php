<?hh

$s = shape(
  'x' => 4
);

var_dump(Shapes::idx($s, 'x'));
var_dump(Shapes::idx($s, 'x', 42));
var_dump(Shapes::idx($s, 'y'));
var_dump(Shapes::idx($s, 'y', 42));

class C {
  const FOO = 4;
}

$t = shape(
  C::FOO => 5
);

Shapes::idx($t, C::FOO);
