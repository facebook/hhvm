<?hh

function gen() {
    yield function() {};
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();

echo "Done!";
}
