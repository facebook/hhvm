<?hh

<<__EntryPoint>>
function test() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    include '1480-1.inc';
  } else {
    include '1480-2.inc';
  }

  include '1480-classes.inc';

  $c = new C();
  $c->foo();
}
