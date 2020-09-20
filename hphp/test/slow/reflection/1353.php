<?hh

<<__EntryPoint>>
function main_1353() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'c' => '1353-1.inc',
        'm' => '1353-2.inc',
      ],
    ],
    __DIR__.'/',
  );

$r1 = new ReflectionClass('C');
$r2 = new ReflectionMethod('M', 'foo');
var_dump($r1->getName());
var_dump($r2->getName());
}
