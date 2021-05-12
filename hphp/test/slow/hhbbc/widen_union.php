<?hh

function some_string() : string {
  return __hhvm_intrinsics\launder_value('');
}
function some_int() : int {
  return __hhvm_intrinsics\launder_value(0);
}

function foo() {
  $r = vec[shape('key' => dict[])];
  while (__hhvm_intrinsics\launder_value(false)) {
    $r[some_int()]['key'][some_string()] = $r[some_int()];
  }
  return $r;
}

<<__EntryPoint>>
function main() {
  var_dump(foo());
}
