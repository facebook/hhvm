<?hh

function gen() {
    yield [] => 1;
}
<<__EntryPoint>> function main() {
$gen = gen();
$gen->next();
var_dump($gen->key());
var_dump($gen->current());
}
