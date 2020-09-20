<?hh

function gen() {
    die('Test');
    yield; // force generator
}

function function_with_3_args() {
    $gen = gen();
    $gen->rewind();
}

function function_with_4_args() {
    function_with_3_args(4, 5, 6);
}

function outerGen() {
    function_with_4_args(0, 1, 2, 3);
    yield; // force generator
}
<<__EntryPoint>> function main(): void {
$outerGen = outerGen();
$outerGen->rewind();
}
