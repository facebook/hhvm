<?hh

function gen(array $array) {
    foreach ($array as $value) {
        yield $value;
    }
}
<<__EntryPoint>> function main(): void {
$gen = gen(varray['Foo', 'Bar']);
$gen->next();
var_dump($gen->current());

// generator is closed here, without running SWITCH_FREE
}
