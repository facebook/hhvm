<?hh

function sprintf_wrapper($format, ...$args) {
  return vsprintf($format, $args);
}

function test_builtin() {
  $format = '%s %d %s';
  $args = varray['a', 10, 'b'];

  var_dump(sprintf_wrapper($format, ...$args));
  var_dump(sprintf($format, ...$args));
}


<<__EntryPoint>>
function main_unpack_call_builtin() {
test_builtin();
}
