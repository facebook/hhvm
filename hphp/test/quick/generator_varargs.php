<?hh

function gen_func_num_args_simple() {
  yield func_num_args();
}

function gen_func_num_args_arg($arg) {
  yield func_num_args();
}


function test(string $generator, array $args) {
  $gen = call_user_func_array($generator, $args);
  foreach ($gen as $val) {
    var_dump($val);
  }
}

function test_num_args(string $type, array $extra_args, mixed ...$more_args) {
  test(
    'gen_func_num_args_'.$type,
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
