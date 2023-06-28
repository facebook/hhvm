<?hh

<<__EntryPoint>>
function test() :mixed{
  if (__hhvm_intrinsics\launder_value(1)) {
    include 'bug63976-1.inc';
  }
  if (__hhvm_intrinsics\launder_value(1)) {
    include 'bug63976-2.inc';
  }
  $bar = new Bar();
  var_dump($bar->table);
}
