<?hh
<<__EntryPoint>> function main(): void {
HH\autoload_set_paths(
  array(
    'class' => array(
      'foo' => 'autoload_record_foo.inc',
    ),
  ),
  __DIR__.'/',
);

$x = Foo['x'=>10];
$y = $x['x'];
var_dump($y);
}
