<?hh

function func_num_args_simple() {
  return func_num_args();
}

function func_num_args_arg($arg) {
  return func_num_args();
}


function test(string $generator, array $args) {
  var_dump(call_user_func_array($generator, $args));
}

function test_num_args(string $type, array $extra_args, mixed ...$more_args) {
  test(
    'func_num_args_'.$type,
    array_merge(
      $more_args,
      $extra_args
    )
  );
}

$extra_args_set = array(
  array(),
  array('hello'),
  array('hello', 47),
);

foreach ($extra_args_set as $extra_args) {
  test_num_args('simple', $extra_args);

  test_num_args('arg', $extra_args, 'defined_arg');
}
