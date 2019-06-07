<?hh

function gen() {
    fn();
    yield;
}

function fn() {
    exit('Done');
}
<<__EntryPoint>> function main() {
$gen = gen();
$gen->next();
$gen->current();
}
