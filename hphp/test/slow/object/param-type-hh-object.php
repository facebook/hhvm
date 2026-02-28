<?hh

function test(HH\object $obj) {
  var_dump(get_class($obj));
}

<<__EntryPoint>>
function main() {
  test(new stdClass());
  test("invalid");
}
