<?hh

class A {
  public static $x = 123;
}

function test() {
  $c = __hhvm_intrinsics\launder_value('A');
  $p = __hhvm_intrinsics\launder_value('x');
  $c::$$p++;
  var_dump($c::$$p);
}


<<__EntryPoint>>
function main_static_prop_dynamic_incdec() {
test();
}
