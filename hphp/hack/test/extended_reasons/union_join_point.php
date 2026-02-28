<?hh
class A {}
class B {}

function foo(bool  $p, A $x, B $y): A {
  if($p){
    $z = $x;
  } else{
    $z = $y;
  }

  return $z;
}
