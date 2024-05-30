<?hh
class A {}
class B {}
class C {}

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


function union_l((A | B) $ab): C {
  return $ab;
}

function union_r(C $c): (A|B) {
  return $c;
}
