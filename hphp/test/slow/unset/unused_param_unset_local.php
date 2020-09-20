<?hh

function test($bar) {
  var_dump(isset(__hhvm_intrinsics\launder_value($foo)));
}

<<__EntryPoint>>
function main() {
  test("bar");
}
