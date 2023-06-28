<?hh

abstract enum class I : mixed {
  const type TypeI = C::T;
  abstract const type TypeAbsI;
}

abstract enum class C : mixed extends I {
  const type T = int;
  abstract const type X as int;
}

<<__EntryPoint>>
function main_reflect_type_constant() :mixed{
echo '=== get_class_constants ===' . PHP_EOL;
var_dump(get_class_constants(C::class));

$rc = new ReflectionClass(C::class);

echo '=== ReflectionClass::getConstants ===' . PHP_EOL;
var_dump($rc->getConstants());

echo '=== ReflectionClass::getAbstractConstantsNames ===' . PHP_EOL;
var_dump($rc->getAbstractConstantNames());

echo "=== ReflectionClass::hasConstant('T') ===" . PHP_EOL;
var_dump($rc->hasConstant('T'));

echo "=== ReflectionClass::hasConstant('X') ===" . PHP_EOL;
var_dump($rc->hasConstant('X'));

echo "=== ReflectionClass::getConstant('T') ===" . PHP_EOL;
var_dump($rc->getConstant('T'));

echo "=== ReflectionClass::getConstant('X') ===" . PHP_EOL;
var_dump($rc->getConstant('X'));

echo "=== ReflectionClass::getTypeConstants ===" . PHP_EOL;
var_dump(array_map(strval<>, $rc->getTypeConstants()));
}
