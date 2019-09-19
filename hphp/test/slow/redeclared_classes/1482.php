<?hh

class A {
}
class B extends A {
  function meh() {
    return $this;
  }
}
class C extends B {
  function work() {
    echo "WORK
";
  }
}
function test() {
  $x = new C;
  $x->meh()->work();
}

<<__EntryPoint>>
function main_1482() {
  if (__hhvm_intrinsics\launder_value(false)) {
    include '1482.inc';
  }
  test();
}
