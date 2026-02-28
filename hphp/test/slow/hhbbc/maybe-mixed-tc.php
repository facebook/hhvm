<?hh

function test(Disable $c) :mixed{
  var_dump($c);
}
<<__EntryPoint>>
function main_entry(): void {

  if (__hhvm_intrinsics\launder_value(true)) {
    require 'maybe-mixed-tc1.inc';
  } else {
    require 'maybe-mixed-tc2.inc';
  }
  test(null);
}
