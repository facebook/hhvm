<?hh

$s = shape(
  'x' => 4
);

Shapes::removeKey($s, 'y');
var_dump($s);
Shapes::removeKey($s, 'x');
var_dump($s);

class C {
  const FOO = 4;
}

$t = shape(
  C::FOO => 5
);

Shapes::removeKey($t, C::FOO);
