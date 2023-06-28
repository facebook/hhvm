<?hh
function gen($o)
:AsyncGenerator<mixed,mixed,void>{
    yield 'foo';
    $o->fatalError();
}
<<__EntryPoint>> function main(): void {
foreach(gen(new stdClass()) as $value)
    echo $value, "\n";
}
