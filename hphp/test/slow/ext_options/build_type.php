<?hh

$build = ini_get('hhvm.build_type');
var_dump(is_string($build));
var_dump(strlen($build) > 0);
var_dump(
  in_array(
    $build,
    ['Debug', 'Release', 'Release with asserts'],
  )
);
