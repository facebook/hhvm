<?hh

function foo(int $a = MY_CONSTANT): void {}
function bar(bool $b): void {}

class A {
  const CLASS_CONST = "Hi";

  public function baz(string $s = A::CLASS_CONST): void {}
  public function herp(string $s = "Hi"): void {}
  public function derp(int $a = 0, int $b = MY_CONSTANT): void {}
}

function burp(string $s = A::CLASS_CONST): void {}


const MY_CONSTANT = 4;
<<__EntryPoint>>
function main_reflection_parameter_is_default_constant_value() :mixed{

$f = new \ReflectionFunction('foo');
foreach ($f->getParameters() as $i => $param) {
  var_dump($param->isDefaultValueConstant());
}
$f = new \ReflectionFunction('bar');
foreach ($f->getParameters() as $i => $param) {
  var_dump($param->isDefaultValueConstant()); // no default value, return NULL
}
$m = new \ReflectionMethod('A', 'baz');
foreach ($m->getParameters() as $i => $param) {
  var_dump($param->isDefaultValueConstant());
}
$m = new \ReflectionMethod('A', 'herp');
foreach ($m->getParameters() as $i => $param) {
  var_dump($param->isDefaultValueConstant());
}
$m = new \ReflectionMethod('A', 'derp');
foreach ($m->getParameters() as $i => $param) {
  var_dump($param->isDefaultValueConstant());
}
$f = new \ReflectionFunction('burp');
foreach ($f->getParameters() as $i => $param) {
  var_dump($param->isDefaultValueConstant());
}
}
