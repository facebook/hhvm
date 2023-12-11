<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
    yield vec[] => 1;
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
var_dump($gen->key());
var_dump($gen->current());
}
