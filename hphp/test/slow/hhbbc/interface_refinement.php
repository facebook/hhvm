<?hh

interface I1 {}

class A {}
class C extends A implements I1 {}
class D extends A implements I1 {}

function h(I1 $x) :mixed{
  return $x;
}

function cc() :mixed{
  return h(new C);
}

function g() :mixed{
  if (__hhvm_intrinsics\launder_value(0))
    $c = h(cc());
  else
    $c = new D;

  var_dump(() ==> { return $c; });
  return $c;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(g());
}
