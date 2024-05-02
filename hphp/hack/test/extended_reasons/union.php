<?hh
class A {}
class B {}

function bar(A $x): A {
  return $x;
}

function foo(bool  $p, A $x, B $y): A {
  if($p){
    $z = $x;
  } else{
    $z = $y;
  }

  return $z;
}
