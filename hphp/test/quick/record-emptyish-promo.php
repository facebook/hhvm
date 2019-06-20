<?hh
record Foo {
    x: ?int,
}

$foo = Foo['x' => null];
$foo['x'][3] = 'abc';
var_dump($foo['x']);
