<?hh

class A extends Exception {
  public $a = 1;
}
function test() {
  try {
    throw new A;
  } catch (A $e) {
    echo $e->a, '
';
  }
}

<<__EntryPoint>>
function main_1473() {
  if (__hhvm_intrinsics\launder_value(0)) {
    include '1473.inc';
  }
  test();
}
