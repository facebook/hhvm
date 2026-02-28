<?hh

function test($bar) :mixed{
  var_dump(isset(__hhvm_intrinsics\launder_value($foo)));
}

<<__EntryPoint>>
function main() :mixed{
  test("bar");
}
