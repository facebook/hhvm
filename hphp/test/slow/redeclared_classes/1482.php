<?hh

class A {
}
class B extends A {
  function meh() :mixed{
    return $this;
  }
}
class C extends B {
  function work() :mixed{
    echo "WORK
";
  }
}
function test() :mixed{
  $x = new C;
  $x->meh()->work();
}

<<__EntryPoint>>
function main_1482() :mixed{
  if (__hhvm_intrinsics\launder_value(false)) {
    include '1482.inc';
  }
  test();
}
