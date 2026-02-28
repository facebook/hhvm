<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
    // execution is suspended here, so the following never gets run:
    echo "Foo";
    // trigger a generator
    yield;
}
<<__EntryPoint>> function main(): void {
$generator = gen();
var_dump($generator is Generator);
}
