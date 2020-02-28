<?hh

function foo(MyVector $a) {
  var_dump($a);
}


<<__EntryPoint>>
function main_autoload_alias() {
$base = dirname(__FILE__).'/';
$map = darray[
  'type'    => darray['myvector' => 'autoload_alias.inc'],
  'failure' => function ($kind, $name) {
    echo "Loading $name?!\n";
}];
\HH\autoload_set_paths($map, $base);

foo(Vector { });
}
