<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
    yield;
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
$gen->send(NULL);

echo "DONE";
}
