<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
    yield 'foo';
    throw new Exception('test');
    yield 'bar';
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
var_dump($gen->current());

try {
    $gen->next();
} catch (Exception $e) {
    echo 'Caught exception with message "', $e->getMessage(), '"', "\n";
}

var_dump($gen->current());
}
