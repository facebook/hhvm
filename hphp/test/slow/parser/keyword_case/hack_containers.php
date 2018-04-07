<?hh

Function test_literals() {
  var_dump(Vec[1]);
  var_dump(Dict['a' => 1]);
  var_dump(Keyset[1]);
  var_dump(Shape('a' => 1));
  var_dump(Tuple(1, 2));
}

function test_casts($x) {
  var_dump(VEC($x));
  var_dump(DICT($x));
  var_dump(KEYSET($x));
}

test_literals();
test_casts(Array('b' => 52));
