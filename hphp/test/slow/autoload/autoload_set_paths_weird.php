<?hh
class A {}

<<__EntryPoint>>
function main_autoload_set_paths_weird() {
var_dump(HH\autoload_set_paths(false, ''));
var_dump(HH\autoload_set_paths(1, ''));
;
var_dump(HH\autoload_set_paths(new A(), ''));
var_dump(HH\autoload_set_paths(STDOUT, ''));

$map = Map {
  'function' => darray[],
  'failure' => function ($kind, $name) {
    echo "Autoload $kind $name\n";
}};
var_dump(HH\autoload_set_paths($map, ''));
b();
}
