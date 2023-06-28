<?hh

function returns_true1() :mixed{ return true; }
function returns_true2() :mixed{ return returns_true1(); }

class A {
  public function foobar() :mixed{ return vec["a"]; }
}
class B {
  public function foobar() :mixed{
    if (returns_true2()) throw Exception();
    return dict["d" => 1];
  }
}
class C {
  public function foobar() :mixed{
    return keyset[__hhvm_intrinsics\launder_value(1)];
  }
}

function baz() :mixed{
  $a = __hhvm_intrinsics\launder_value(new A());
  return $a->foobar();
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(baz());
}
