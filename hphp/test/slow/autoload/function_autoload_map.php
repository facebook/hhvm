<?hh

namespace {
$map = ['function' => [], 'failure' => function () { var_dump(func_get_args()); }];
HH\autoload_set_paths($map, '');
}

namespace foo {
  test();
}
