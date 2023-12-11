<?hh

function sprintf_wrapper($format, ...$args) :mixed{
  return vsprintf($format, $args);
}

function test_builtin() :mixed{
  $format = '%s %d %s';
  $args = vec['a', 10, 'b'];

  var_dump(sprintf_wrapper($format, ...$args));
  var_dump(sprintf($format, ...$args));
}


<<__EntryPoint>>
function main_unpack_call_builtin() :mixed{
test_builtin();
}
