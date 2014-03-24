<?hh
function main() {
  $x = Vector {1};
  $y = $x->toImmVector();
  $x[0] += 2;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Vector {1};
  $y = $x->toImmVector();
  ++$x[0];
  var_dump($y);
  unset($y);
  unset($x);

  $x = Vector {null};
  $y = $x->toImmVector();
  $x[0][] = 73;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Vector {null};
  $y = $x->toImmVector();
  $x[0][42] = 73;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Vector {null};
  $y = $x->toImmVector();
  $x[0]->prop = 73;
  var_dump($y);
  $x[0] = null;
  unset($y);
  unset($x);

  $x = Vector {'foo'};
  $y = $x->toImmVector();
  $x[0][0] = 'g';
  var_dump($y);
  unset($y);
  unset($x);

  $x = Vector {array(1)};
  $y = $x->toImmVector();
  unset($x[0][0]);
  var_dump($y);
  unset($y);
  unset($x);

  $x = Vector {array(1)};
  $y = $x->toImmVector();
  $x[0][] = 2;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Vector {array(1)};
  $y = $x->toImmVector();
  $x[0][1] = 2;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Vector {Vector {1}};
  $y = $x->toImmVector();
  ++$x[0][0];
  var_dump($y);
  unset($y);
  $x->pop();
  unset($x);

  $x = Vector {Vector {1}};
  $y = $x->toImmVector();
  $x[0][0] += 2;
  var_dump($y);
  unset($y);
  $x->pop();
  unset($x);

  $x = Vector {Vector {1}};
  $y = $x->toImmVector();
  $x[0][] = 2;
  var_dump($y);
  unset($y);
  $x->pop();
  unset($x);

  $x = Vector {Vector {1}};
  $y = $x->toImmVector();
  $x[0][0] = 2;
  var_dump($y);
  unset($y);
  $x->pop();
  unset($x);
}
main();

