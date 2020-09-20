<?hh
function gen($o)
{
    yield 'foo';
    $o->fatalError();
}
<<__EntryPoint>> function main(): void {
foreach(gen(new stdClass()) as $value)
    echo $value, "\n";
}
