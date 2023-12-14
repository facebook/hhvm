<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
    exit('Test');
    yield; // force generator
}

function function_with_3_args() :mixed{
    $gen = gen();
    $gen->rewind();
}

function function_with_4_args() :mixed{
    function_with_3_args(4, 5, 6);
}

function outerGen() :AsyncGenerator<mixed,mixed,void>{
    function_with_4_args(0, 1, 2, 3);
    yield; // force generator
}
<<__EntryPoint>> function main(): void {
$outerGen = outerGen();
$outerGen->rewind();
}
