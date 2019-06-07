<?hh

function gen() {
    $a = 1;
    yield $a;
}
<<__EntryPoint>> function main() {
@eval('abc');

$values = gen();
$values->next();

echo "===DONE===\n";
}
