<?hh

function foo() {
  $x = 123;
  var_dump((string)$x);
  $y = -456;
  var_dump((string)$y);
}

foo();
