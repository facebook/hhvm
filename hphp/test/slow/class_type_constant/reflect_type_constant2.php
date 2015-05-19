<?hh

interface I {
  const type TypeI = C::T;
  abstract const type TypeAbsI;
}

abstract class C implements I {
  const type T = int;
  abstract const type X as int;
  const TYPE = 0;
}

$rc = new ReflectionClass(C::class);

echo "=== ReflectionClass::getTypeConstant('T') ===" . PHP_EOL;
var_dump((string)$rc->getTypeConstant('T'));

echo "=== ReflectionClass::getTypeConstant('X') ===" . PHP_EOL;
var_dump((string)$rc->getTypeConstant('X'));

echo "=== ReflectionClass::hasTypeConstant('TypeAbsI') ===" . PHP_EOL;
var_dump($rc->hasTypeConstant('TypeAbsI'));

echo "=== ReflectionClass::hasTypeConstant('TYPE') ===" . PHP_EOL;
var_dump($rc->hasTypeConstant('TYPE'));

echo PHP_EOL;

// Test non-interned string
$tc = new ReflectionTypeConstant(C::class, trim(' TypeI '));
echo '=== <C::TypeI>::getDeclaringClass() ===' . PHP_EOL;
var_dump($tc->getDeclaringClass()->getName());

echo '=== <C::TypeI>::getName() ===' . PHP_EOL;
var_dump($tc->getName());

echo '=== <C::TypeI>::getAssignedTypeText() ===' . PHP_EOL;
var_dump($tc->getAssignedTypeText());

echo '=== <C::TypeI>::isAbstract() ===' . PHP_EOL;
var_dump($tc->isAbstract());

echo PHP_EOL;
$tc = new ReflectionTypeConstant(C::class, 'X');
echo '=== <C::X>::getDeclaringClass() ===' . PHP_EOL;
var_dump($tc->getDeclaringClass()->getName());

echo '=== <C::X>::getName() ===' . PHP_EOL;
var_dump($tc->getName());

echo '=== <C::X>::getAssignedTypeText() ===' . PHP_EOL;
var_dump($tc->getAssignedTypeText());

echo '=== <C::X>::isAbstract() ===' . PHP_EOL;
var_dump($tc->isAbstract());

echo PHP_EOL;
echo '=== ReflectionTypeConstant(C, x) ===' . PHP_EOL;
// Type Constants are case sensitive
try {
  new ReflectionTypeConstant(C::class, 'x');
} catch (ReflectionException $e) {
  var_dump($e->getMessage());
}
