<?hh

HH\autoload_set_paths(
  array(
    'record' => array(
      'foo' => 'autoload_record_foo.inc',
    ),
  ),
  __DIR__.'/',
);

$x = Foo['x'=>10];
$y = $x['x'];
var_dump($y);
