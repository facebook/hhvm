<?hh

HH\autoload_set_paths(
  array(
    'function' => array(
      'foo' => 'autoload_foo.inc',
    ),
  ),
  __DIR__.'/',
);

foo();
