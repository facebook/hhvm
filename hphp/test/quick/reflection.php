<?php

print "Test begin\n";

define('SOME_CONSTANT', "some string");

#===============================================================================
# ReflectionFunction.

/**
 * This is f's doc comment.
 */
function &f($a, &$b, $c=null, $d=array(1, 2, SOME_CONSTANT)) {
  static $staticX = 4;
  static $staticY;
  print "In f()\n";
  $staticX++;
  $x = $staticX;
  return $x;
}

$rf = new ReflectionFunction("f");

print "--- getDocComment(\"f\") ---\n";
var_dump($rf->getDocComment());
print "\n";

print "--- getStartLine(\"f\") ---\n";
var_dump($rf->getStartLine());
print "\n";

print "--- getEndLine(\"f\") ---\n";
var_dump($rf->getEndLine());
print "\n";

print "--- getFileName(\"f\") ---\n";
var_dump($rf->getFileName());
print "\n";

print "--- getName(\"f\") ---\n";
var_dump($rf->getName());
print "\n";

print "--- getNumberOfParameters(\"f\") ---\n";
var_dump($rf->getNumberOfParameters());
print "\n";

print "--- getNumberOfRequiredParameters(\"f\") ---\n";
var_dump($rf->getNumberOfRequiredParameters());
print "\n";

print "--- getParameters(\"f\") ---\n";
var_dump($rf->getParameters());
print "\n";

print "--- getStaticVariables(\"f\") ---\n";
var_dump($rf->getStaticVariables());
print "\n";

print "--- isInternal(\"f\") ---\n";
var_dump($rf->isInternal());
print "\n";

print "--- isUserDefined(\"f\") ---\n";
var_dump($rf->isUserDefined());
print "\n";

print "--- returnsReference(\"f\") ---\n";
var_dump($rf->returnsReference());
print "\n";

print "--- export(\"f\") ---\n";
var_dump($rf->export('f', true));
print "\n";

# invoke() can't be used because $b is pass-by-reference.

print "--- invokeArgs(\"f\") ---\n";
$b = "b";
var_dump($rf->invokeArgs(array("a", &$b, "c")));
var_dump($rf->invokeArgs(array("a", &$b, "c")));
print "\n";

/**
 * This is g's doc comment.
 */
function g($a=null, $b=array(1, 2, 3), $c=SOME_CONSTANT) {
  print "In g($a, $b, $c)\n";
}

$rg = new ReflectionFunction("g");

print "--- invoke(\"g\") ---\n";
var_dump($rg->invoke("a", "b"));
var_dump($rg->invoke("a", "b"));
print "\n";

print "--- export(\"g\") ---\n";
var_dump($rf->export('g', true));
print "\n";

#===============================================================================
# ReflectionClass.

interface H {
  public function methH();
}
interface I {
  public function methI();
}
interface J {
  public function methJ();
}
interface K extends I, J {
  public function methK();
}
interface L {}

class A implements H {
  public function methH() {}
  protected function methA() {}
}
/**
 * This is B's doc comment.
 */
class B extends A implements I, K {
  const C0 = "B::C0";
  const C1 = "B::C1";
  static $s0 = 42;
  static $s1 = "hello";
  static $s2;
  private $p0 = 1;
  protected $p1 = 2;
  public $p2 = 3;
  public $p3;
  static public function smethB0() {}
  static private function smethB1() {}
  public function methI() {}
  public function methJ() {}
  public function methK() {}
  private function methB() {}
}
class C {}

$rb = new ReflectionClass("B");

print "--- export() ---\n";
var_dump($rb->export('B', true));
print "\n";

print "--- getConstant() ---\n";
var_dump($rb->getConstant('C0'));
var_dump($rb->getConstant('C1'));
print "\n";

print "--- getConstants() ---\n";
var_dump($rb->getConstants());
print "\n";

print "--- getConstructor() ---\n";
var_dump($rb->getConstructor());
print "\n";

print "--- getDocComment() ---\n";
var_dump($rb->getDocComment());
print "\n";

print "--- getStartLine() ---\n";
var_dump($rb->getStartLine());
print "\n";

print "--- getEndLine() ---\n";
var_dump($rb->getEndLine());
print "\n";

print "--- getFileName() ---\n";
var_dump($rb->getFileName());
print "\n";

print "--- getInterfaceNames() ---\n";
var_dump($rb->getInterfaceNames());
print "\n";

print "--- getInterfaces() ---\n";
# Very verbose.
#var_dump(
          $rb->getInterfaces()
#         )
          ;
print "\n";

print "--- getMethod() ---\n";
print "\n";

print "--- getMethods() ---\n";
# Very verbose.
#var_dump(
          $rb->getMethods()
#         )
          ;
print "\n";

print "--- getModifiers() ---\n";
var_dump($rb->getModifiers());
print "\n";

print "--- getName() ---\n";
var_dump($rb->getName());
print "\n";

print "--- getParentClass() ---\n";
var_dump($rb->getParentClass());
print "\n";

print "--- getProperties() ---\n";
# Very verbose.
#var_dump(
          $rb->getProperties()
#         )
          ;
print "\n";

print "--- getProperty() ---\n";
# Very verbose.
#var_dump(
          $rb->getProperty('p0')
#         )
          ;
print "\n";

print "--- getStaticProperties() ---\n";
# Very verbose.
#var_dump(
          $rb->getStaticProperties()
#         )
          ;
print "\n";

print "--- setStaticPropertyValue() ---\n";
var_dump($rb->setStaticPropertyValue('s0', 'new value for s0'));
print "\n";

print "--- getStaticPropertyValue() ---\n";
var_dump($rb->getStaticPropertyValue('s0'));
var_dump($rb->getStaticPropertyValue('s4'));
print "\n";

print "--- hasConstant() ---\n";
var_dump($rb->hasConstant('C0'));
var_dump($rb->hasConstant('C4'));
print "\n";

print "--- hasMethod() ---\n";
var_dump($rb->hasMethod('methB'));
var_dump($rb->hasMethod('methX'));
print "\n";

print "--- hasProperty() ---\n";
var_dump($rb->hasProperty('p0'));
var_dump($rb->hasProperty('p4'));
print "\n";

print "--- implementsInterface() ---\n";
var_dump($rb->implementsInterface('H'));
var_dump($rb->implementsInterface('L'));
print "\n";

print "--- isAbstract() ---\n";
var_dump($rb->isAbstract());
print "\n";

print "--- isFinal() ---\n";
var_dump($rb->isFinal());
print "\n";

print "--- isInstance() ---\n";
var_dump($rb->isInstance(new B));
var_dump($rb->isInstance(new C));
print "\n";

print "--- isInstantiable() ---\n";
var_dump($rb->isInstantiable());
print "\n";

print "--- isInterface() ---\n";
var_dump($rb->isInterface());
print "\n";

print "--- isInternal() ---\n";
var_dump($rb->isInternal());
print "\n";

print "--- isIterateable() ---\n";
var_dump($rb->isIterateable());
print "\n";

print "--- isSubclassOf() ---\n";
var_dump($rb->isSubclassOf('A'));
var_dump($rb->isSubclassOf('C'));
print "\n";

print "--- isUserDefined() ---\n";
var_dump($rb->isUserDefined());
print "\n";

print "--- newInstance() ---\n";
var_dump($rb->newInstance());
print "\n";

print "--- newInstanceArgs() ---\n";
var_dump($rb->newInstanceArgs());
print "\n";

print "--- get_defined_functions() ---\n";
$a = get_defined_functions()["user"];
sort($a);
var_dump($a);

print "--- get_defined_constants() ---\n";
$a = get_defined_constants();
print "SOME_CONSTANT: " . $a["SOME_CONSTANT"] . "\n";
if (isset($a["ANOTHER_CONSTANT"])) {
  print "ANOTHER_CONSTANT: ".$a["ANOTHER_CONSTANT"]."\n";
}
define('ANOTHER_CONSTANT', "some other string");
$a = get_defined_constants();
print "SOME_CONSTANT: " . $a["SOME_CONSTANT"] . "\n";
print "ANOTHER_CONSTANT: ".$a["ANOTHER_CONSTANT"]."\n";

print "--- get_declared_classes() ---\n";
$a = array_flip(get_declared_classes());
$classes = array("A", "B", "C");
foreach ($classes as $c) {
  if (isset($a[$c])) {
    print "Found class $c\n";
  } else {
    print "Missing class $c\n";
  }
}

print "--- get_declared_interfaces() ---\n";
$a = array_flip(get_declared_interfaces());
$interfaces = array("H", "I", "J", "K", "L");
foreach ($interfaces as $i) {
  if (isset($a[$i])) {
    print "Found interface $i\n";
  } else {
    print "Missing interface $i\n";
  }
}

print "Test end\n";
