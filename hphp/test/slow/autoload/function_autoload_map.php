<?hh

namespace {
  $map = darray['function' => [], 'failure' => function ($kind, $name) {
    echo "Autoload $kind $name\n";
}];
HH\autoload_set_paths($map, '');
}

namespace foo {
  test();
}
