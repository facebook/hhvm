<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
    yield vec[1, 2, 3] => vec[4, 5, 6];
    $a = new stdClass(); $a->a = "b";
    $b = new stdClass(); $b->b = "a";
    yield $a => $b;
    yield 3.14 => 2.73;
    yield false => true;
    yield true => false;
    yield null => null;
}
<<__EntryPoint>> function main(): void {
foreach (gen() as $k => $v) {
    var_dump($k, $v);
}
}
