<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
    yield function() {};
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();

echo "Done!";
}
