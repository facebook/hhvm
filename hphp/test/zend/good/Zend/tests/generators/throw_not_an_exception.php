<?hh

function gen() {
    yield;
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
$gen->throw(new stdClass);
}
