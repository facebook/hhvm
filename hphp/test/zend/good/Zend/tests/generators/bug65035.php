<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
    fn();
    yield;
}

function fn() :mixed{
    exit('Done');
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
$gen->current();
}
