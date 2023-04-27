<?hh

function foo1(dict $d) {
  $d['a'] = null ^ (string)__hhvm_intrinsics\launder_value("abc");
  return $d;
}

<<__EntryPoint>>
function main() {
  if (__hhvm_intrinsics\launder_value(false)) {
    var_dump(foo1(dict[]));
  } else {
    echo "Done\n";
  }
}
