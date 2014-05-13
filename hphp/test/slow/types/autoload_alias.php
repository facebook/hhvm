<?hh

$base = dirname(__FILE__).'/';
$map = array(
  'type'    => array('myvector' => 'autoload_alias.inc'),
  'failure' => function ($kind, $name) {
    echo "Loading $name?!\n";
});
\HH\autoload_set_paths($map, $base);

function foo(MyVector $a) {
  var_dump($a);
}

foo(Vector { });
