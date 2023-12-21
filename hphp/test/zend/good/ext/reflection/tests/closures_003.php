<?hh
<<__EntryPoint>> function main(): void {
$closure = function($a, $b = 0) { };

$method = new ReflectionMethod ($closure, '__invoke');
$params = $method->getParameters ();
unset ($method);
$method = $params[0]->getDeclaringFunction ();
unset ($params);
echo $method->getName ()."\n";

$parameter = new ReflectionParameter (vec[$closure, '__invoke'], 'b');
$method = $parameter->getDeclaringFunction ();
unset ($parameter);
echo $method->getName ()."\n";

echo "===DONE===\n";
}
