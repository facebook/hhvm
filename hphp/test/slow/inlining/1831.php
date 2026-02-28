<?hh

function h() :mixed{
 include '1831.inc';
}
function f($a, $b, $c) :mixed{
 return h();
}
function g($a, $b, $c) :mixed{
  $__lval_tmp_0 = $a;
  $a++;
  $__lval_tmp_1 = $b;
  $b++;
  $__lval_tmp_2 = $a;
  $a++;
  return f($__lval_tmp_0, $__lval_tmp_1+ $__lval_tmp_2, $c);
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
