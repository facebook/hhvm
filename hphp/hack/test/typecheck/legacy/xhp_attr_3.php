<?hh
function test($obj) {
  // Using ->: syntax for writing is not supported for now
  $obj->:foo = 123;
}
