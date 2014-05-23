<?hh

var_dump(HH\autoload_set_paths(false, ''));
var_dump(HH\autoload_set_paths(1, ''));
class A {};
var_dump(HH\autoload_set_paths(new A(), ''));
var_dump(HH\autoload_set_paths(STDOUT, ''));

$map = Map {
  'function' => [],
  'failure' => function ($kind, $name) {
    echo "Autoload $kind $name\n";
}};
var_dump(HH\autoload_set_paths($map, ''));
b();
