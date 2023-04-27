<?hh

function returns_true1() { return true; }
function returns_true2() { return returns_true1(); }

class A {
  public function foobar() { return vec["a"]; }
}
class B {
  public function foobar() {
    if (returns_true2()) throw Exception();
    return dict["d" => 1];
  }
}
class C {
  public function foobar() {
    return keyset[__hhvm_intrinsics\launder_value(1)];
  }
}

function baz() {
  $a = __hhvm_intrinsics\launder_value(new A());
  return $a->foobar();
}

<<__EntryPoint>>
function main() {
  var_dump(baz());
}
