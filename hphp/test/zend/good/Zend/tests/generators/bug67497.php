<?hh

function gen() {
    $a = 1;
    yield $a;
}
<<__EntryPoint>> function main(): void {
@eval('abc');

$values = gen();
$values->next();

echo "===DONE===\n";
}
