<?hh

<<__EntryPoint>>
function foo() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    include '1723.inc';
  }

  include '1723.b.inc';

  $obj = new B(1, 2, 3);
  var_dump($obj);
}
