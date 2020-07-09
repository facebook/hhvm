<?hh

<<__EntryPoint>>
function main(): void {
  if (__hhvm_intrinsics\launder_value(true)) {
    include '1475-1.inc';
  } else {
    include '1475-2.inc';
  }
  include '1475-classes.inc';
  $obj = new child1;
  echo ($obj is Exception) ? "Passed
  " : "Failed
  ";
}
