<?hh
<<__EntryPoint>> function main(): void {
$closure = function($a, $b = 0) { };

$method = new ReflectionMethod ($closure, '__invoke');
$params = $method->getParameters ();
$method = null;
$method = $params[0]->getDeclaringFunction ();
$params = null;
echo $method->getName ()."\n";

$parameter = new ReflectionParameter (vec[$closure, '__invoke'], 'b');
$method = $parameter->getDeclaringFunction ();
$parameter = null;
echo $method->getName ()."\n";

echo "===DONE===\n";
}
