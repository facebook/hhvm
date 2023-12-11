<?hh

class Test {
    function __invoke($a, $b = 0) :mixed{ }
}
<<__EntryPoint>> function main(): void {
$rm = new ReflectionMethod(new Test, '__invoke');
var_dump($rm->getName());
var_dump($rm->getNumberOfParameters());
var_dump($rm->getNumberOfRequiredParameters());

$rp = new ReflectionParameter(vec[new Test, '__invoke'], 0);
var_dump($rp->isOptional());

$rp = new ReflectionParameter(vec[new Test, '__invoke'], 1);
var_dump($rp->isOptional());

echo "===DONE===\n";
}
