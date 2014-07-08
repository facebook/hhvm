<?hh

class X {
  const string X1 = 'field1';
  const string X2 = 'field2';
}

type myshape = shape(X::X1 => int, X::X2 => bool);

function test(): myshape {
  $x = shape(X::X1 => 1, X::X2 => true);
  return $x;
}

function test2(): myshape {
  // The typechecker won't like this but we shouldn't care
  return shape('field1' => 1, 'field2' => true);
}


function main(): void {
  $y = test();
  var_dump($y[X::X1]);
  var_dump($y);

  var_dump(test2());
}

main();
