<?hh

function gen($foo, $bar) {
    yield $foo;
    yield $bar;
}
<<__EntryPoint>> function main(): void {
$gen = call_user_func(fun('gen'), 'bar', 'foo');
foreach ($gen as $value) {
    var_dump($value);
}
}
