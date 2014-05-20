<?hh

HH\autoload_set_paths(false, '');
HH\autoload_set_paths(1, '');
class A {};
HH\autoload_set_paths(new A, '');
HH\autoload_set_paths(STDOUT, '');

$map = Map {
  'function' => [],
  'failure' => function ($kind, $name) {
    echo "Autoload $kind $name\n";
}};
HH\autoload_set_paths($map, '');
b();
