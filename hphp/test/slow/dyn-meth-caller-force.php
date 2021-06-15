<?hh

class Foo { private function bar() { echo "bar\n"; } }

<<__EntryPoint>>
function main() {
  $c = __hhvm_intrinsics\launder_value('Foo');
  $m = __hhvm_intrinsics\launder_value('bar');

  $mc = HH\dynamic_meth_caller_force($c, $m);
  $mc(new Foo);

  $mc2 = __hhvm_intrinsics\launder_value($mc);
  $mc2(new Foo);
}
