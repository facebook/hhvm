<?hh

class C {
  const FOO = 4;
}


<<__EntryPoint>>
function main_idx() :mixed{
  $s = shape('x' => 4);

  var_dump(Shapes::idx($s, 'x'));
  var_dump(Shapes::idx($s, 'x', 42));
  var_dump(Shapes::idx($s, 'y'));
  var_dump(Shapes::idx($s, 'y', 42));

  $t = shape(C::FOO => 5);

  var_dump(Shapes::idx($s, C::FOO));
  var_dump(Shapes::idx($s, C::FOO, 42));
  var_dump(Shapes::idx($t, C::FOO));
  var_dump(Shapes::idx($t, C::FOO, 42));
}
