<?hh
function foo(string $a, int $b, float $c, bool $d, Exception &$e,
             callable $f = null, resource $g = null, $noType = 'whatever') {}

$reflectionFunction = new ReflectionFunction('foo');
foreach ($reflectionFunction->getParameters() as $parameter) {
  echo 'name: ';
  var_dump($parameter->getName());
  echo 'position: ';
  var_dump($parameter->getPosition());
  echo 'canBePassedByValue: ';
  var_dump($parameter->canBePassedByValue());
  echo 'isPassedByReference: ';
  var_dump($parameter->isPassedByReference());
  echo 'hasType: ';
  var_dump($parameter->hasType());
  echo 'getType: ';
  var_dump($parameter->getType());
  echo 'type isBuiltin: ';
  var_dump($parameter->hasType() ? $parameter->getType()->isBuiltin() : false);
  echo 'type allowsNull: ';
  var_dump($parameter->hasType() ? $parameter->getType()->allowsNull() : false);
  echo 'type hint: ';
  var_dump($parameter->hasType() ? $parameter->getType()->__toString() : false);
  echo 'isArray: ';
  var_dump($parameter->isArray());
  echo 'isCallable: ';
  var_dump($parameter->isCallable());
  echo 'isOptional: ';
  var_dump($parameter->isOptional());
  echo 'isVariadic: ';
  var_dump($parameter->isVariadic());
  echo 'allowsNull: ';
  var_dump($parameter->allowsNull());
  echo 'isDefaultValueAvailable: ';
  var_dump($parameter->isDefaultValueAvailable());
  echo 'isDefaultValueConstant: ';
  var_dump($parameter->isDefaultValueAvailable() &&
           $parameter->isDefaultValueConstant());
  echo 'getDefaultValue: ';
  var_dump($parameter->isDefaultValueAvailable()
           ? $parameter->getDefaultValue()
           : 'no default value');
  echo 'getDefaultValueConstantName: ';
  var_dump($parameter->isDefaultValueAvailable()
           ? $parameter->getDefaultValueConstantName()
           : 'no default value');
  echo PHP_EOL, PHP_EOL;
}
