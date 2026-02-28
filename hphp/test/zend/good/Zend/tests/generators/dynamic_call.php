<?hh

function gen($foo, $bar) :AsyncGenerator<mixed,mixed,void>{
    yield $foo;
    yield $bar;
}
<<__EntryPoint>> function main(): void {
$gen = call_user_func(gen<>, 'bar', 'foo');
foreach ($gen as $value) {
    var_dump($value);
}
}
