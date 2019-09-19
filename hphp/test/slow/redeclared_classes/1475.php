<?hh

if (__hhvm_intrinsics\launder_value(true)) {
  include '1475-1.inc';
} else {
  include '1475-2.inc';
}
class child1 extends base {
}
$obj = new child1;
echo ($obj is Exception) ? "Passed
" : "Failed
";
