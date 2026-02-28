<?hh
function foo() :AsyncGenerator<mixed,mixed,void>{
    throw new Exception;
    yield 1;
    yield 2;
    return 42;
}
<<__EntryPoint>> function main(): void {
$bar = foo();

try {
    $bar->next();
} catch (Exception $e) {
}

var_dump($bar->getReturn());
}
