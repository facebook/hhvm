<?hh
function main() :mixed{
  $x = Vector {1};
  $y = $x->toImmVector();
  $x[0] += 2;
  var_dump($y);
  $y = null;
  $x = null;

  $x = Vector {1};
  $y = $x->toImmVector();
  ++$x[0];
  var_dump($y);
  $y = null;
  $x = null;

  $x = Vector {null};
  $y = $x->toImmVector();
  $x[0] = vec[]; $x[0][] = 73;
  var_dump($y);
  $y = null;
  $x = null;

  $x = Vector {null};
  $y = $x->toImmVector();
  $x[0] = dict[]; $x[0][42] = 73;
  var_dump($y);
  $y = null;
  $x = null;

  $x = Vector {new stdClass()};
  $y = $x->toImmVector();
  $x[0]->prop = 73;
  var_dump($y);
  $x[0] = null;
  $y = null;
  $x = null;

  $x = Vector {'foo'};
  $y = $x->toImmVector();
  $x[0][0] = 'g';
  var_dump($y);
  $y = null;
  $x = null;

  $x = Vector {vec[1]};
  $y = $x->toImmVector();
  unset($x[0][0]);
  var_dump($y);
  $y = null;
  $x = null;

  $x = Vector {vec[1]};
  $y = $x->toImmVector();
  $x[0][] = 2;
  var_dump($y);
  $y = null;
  $x = null;

  $x = Vector {dict[0 => 1]};
  $y = $x->toImmVector();
  $x[0][1] = 2;
  var_dump($y);
  $y = null;
  $x = null;

  $x = Vector {Vector {1}};
  $y = $x->toImmVector();
  ++$x[0][0];
  var_dump($y);
  $y = null;
  $x->pop();
  $x = null;

  $x = Vector {Vector {1}};
  $y = $x->toImmVector();
  $x[0][0] += 2;
  var_dump($y);
  $y = null;
  $x->pop();
  $x = null;

  $x = Vector {Vector {1}};
  $y = $x->toImmVector();
  $x[0][] = 2;
  var_dump($y);
  $y = null;
  $x->pop();
  $x = null;

  $x = Vector {Vector {1}};
  $y = $x->toImmVector();
  $x[0][0] = 2;
  var_dump($y);
  $y = null;
  $x->pop();
  $x = null;
}


<<__EntryPoint>>
function main_vector_buffer_sharing() :mixed{
main();
}
