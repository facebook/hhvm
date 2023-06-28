<?hh

<<__EntryPoint>>
function main_1485() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    include '1485-1.inc';
  } else {
    include '1485-2.inc';
  }
  $obj = new B;
  $obj->f();
  var_dump($obj);
}
