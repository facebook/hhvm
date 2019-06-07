<?hh

function gen() {
    yield;
}
<<__EntryPoint>> function main() {
$gen = gen();
$gen->next();
$gen->send(NULL);

echo "DONE";
}
