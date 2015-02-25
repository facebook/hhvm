<?php

// Add code snippets here
// Call them from MeasureBasicOps.php to collect relative CPU time stats

function function_call($i) {
    return ($i + 1);
}

class MethodCall {
  public static function callStaticMethod($i) {
    return ($i + 1);
  }

  public function callInstanceMethod($i) {
    return ($i + 1);
  }
}

interface ITest0 {
  public function callInterfaceMethod($i);
}

class Class0 implements ITest0 {
  public function callInterfaceMethod ($i) {
    return ($i + 1);
  }
}

interface ITest1 {
}

interface ITest2 {
}

interface ITest3 {
}

interface ITest4 {
}

interface ITest5 {
}

class Miss0 {
  public function instanceofMiss0() {
    $bool = $this instanceof ITest2 ? 1 : 0;
    return ($bool);
  }
}

class Miss1 implements ITest1 {
  public function instanceofMiss1() {
    $bool = $this instanceof ITest2 ? 1 : 0;
    return ($bool);
  }
}

class Hit1 implements ITest2 {
  public function instanceofHit1() {
    $bool = $this instanceof ITest2 ? 1 : 0;
    return ($bool);
  }
}

class Miss4 implements ITest1, ITest3, ITest4, ITest5 {
  public function instanceofMiss4() {
    $bool = $this instanceof ITest2 ? 1 : 0;
    return ($bool);
  }
}

class Hit5 implements ITest1, ITest2, ITest3, ITest4, ITest5 {
  public function instanceofHit5() {
    $bool = $this instanceof ITest2 ? 1 : 0;
    return ($bool);
  }
}

class X {
  private static $x;

  public static final function isX() {
    if (!isset(X::$x)) {
      X::$x = true;
    }
    return (X::$x ? 1 : 0);
  }
}

class Y {
  private static $y = true;

  public static final function isY() {
    return (Y::$y ? 1 : 0);
  }
}

// HHVM has idx() built in, but PHP5 doesn't
if (!function_exists('idx')) {
  function idx($arr, $key, $def = null) {
    return (isset($arr[$key]) ? $arr[$key] : $def);
  }
}

class EmptyArray {
  private $array1 = array();

  public function indexEmpty() {
    return (idx($this->array1, "xyz", 0));
  }
}

class NoHitSmallArray {
  private $array2 = array('abc' => 4);

  public function indexNoHitSmall() {
    return (idx($this->array2, "xyz", 0));
  }
}

class HitSmallArray {
  private $array3 = array('xyz' => 1);

  public function indexHitSmall() {
    return (idx($this->array3, "xyz", 0));
  }
}

class NoHitLargeArray {
  private $array4 = array('a' => 1, 'b' => 2, 'c' => 3, 'd' => 4, 'e' => 5,
                          'f' => 6, 'g' => 7, 'h' => 8, 'i' => 9, 'j' => 10,
                          'k' => 11, 'l' => 12, 'm' => 13, 'n' => 14, 'o' => 15,
                          'p' => 16, 'q' => 17, 'r' => 18, 's' => 19, 't' => 20,
                          'u' => 21, 'v' => 22, 'w' => 23, 'x' => 24, 'y' => 25,
                          'z' => 26);

  public function indexNoHitLarge() {
    return (idx($this->array4, "xyz", 0));
  }
}

class HitLargeArray {
  private $array5 = array('a' => 1, 'b' => 2, 'c' => 3, 'd' => 4, 'e' => 5,
                          'f' => 6, 'g' => 7, 'h' => 8, 'i' => 9, 'j' => 10,
                          'k' => 11, 'l' => 12, 'm' => 13, 'n' => 14, 'o' => 15,
                          'p' => 16, 'q' => 17, 'r' => 18, 's' => 19, 't' => 20,
                          'u' => 21, 'v' => 22, 'w' => 23, 'x' => 24, 'y' => 25,
                          'z' => 26, 'xyz' => 1);

  public function indexHitLarge() {
    return (idx($this->array5, "xyz", 0));
  }
}

class MethodExistsString {
  private $x = 'test';

  public function methodexists() {
    return (method_exists($this->x, '__toString') ? 1 : 0);
  }
}

class MethodExistsClass {
  public function __toString() {
  }

  public function methodexists() {
    return (method_exists($this, '__toString') ? 1 : 0);
  }
}

class MethodNotExistsClass {
  public function methodexists() {
    return (method_exists($this, '__toString') ? 1 : 0);
  }
}

class MethodExistsBaseClass {
  public function __toString() {
  }
}

class MethodExistsDerivedClass extends MethodExistsBaseClass {
  public function methodexists() {
    return (method_exists($this, '__toString') ? 1 : 0);
  }
}
 class MethodNotExistsBaseClass {
   public function baseMethod() {
   }
 }

 class MethodNotExistsDerivedClass extends MethodNotExistsBaseClass {
   public function methodexists() {
     return (method_exists ($this, '__toString') ? 1 : 0);
   }
 }

class FieldAccess {
  public static $staticCounter = 0;
  public $instanceCounter = 0;
  public static $staticString = "";
  public $instanceString = "";

  public static function accessStaticCounter() {
    FieldAccess::$staticCounter++;
    return (FieldAccess::$staticCounter);
  }

  public function accessInstanceCounter() {
    $this->instanceCounter++;
    return ($this->instanceCounter);
  }

  public static function accessStaticString() {
    FieldAccess::$staticString = "hi";
    return (FieldAccess::$staticString == "hi" ? 1 : 0);
   }

  public function accessInstanceString() {
    $this->instanceString = "hi";
    return ($this->instanceString == "hi" ? 1 : 0);
   }
}

class IntArray {
  private $intarray = array();
  public function assignArrayElem() {
    for ($iter = 0; $iter < 1; $iter++) {
     $this->intarray[$iter] = 1;
    }
    return (1);
  }
}

class StringArray {
  private $stringarray = array();
  public function assignArrayElem() {
    for ($iter = 0; $iter < 1; $iter++) {
     $this->stringarray[$iter] = "hi";
    }
    return (1);
  }
}

function testfunc() {
  return (1);
}

class AnonymousFunctions {
  private function useAnonymousFunction($func) {
    return ($func());
  }

  private static function useAnonymousFunctionStatic($func) {
    return ($func());
  }

  public function instanceFunction() {
    $func = function() {
      return (1);
    };
    return ($this->useAnonymousFunction($func));
  }

  public static function staticFunction() {
    $func = function() {
      return (1);
    };
    return (AnonymousFunctions::useAnonymousFunctionStatic($func));
  }

  private function useVariableFunction($funcname) {
    return ($funcname());
  }

  private static function useVariableFunctionStatic($funcname) {
    return ($funcname());
  }

  public function variableFunction() {
    return ($this->useVariableFunction("testfunc"));
  }

  public static function variableFunctionStatic() {
    return (AnonymousFunctions::useVariableFunctionStatic("testfunc"));
  }
}

class ItTestClassArray {
}

class IteratorTestArray {
  private $intArray10 = array();
  private $stringArray10 = array();
  private $objectArray10 = array();
  private $intArray100 = array();
  private $stringArray100 = array();
  private $objectArray100 = array();

  public function __construct() {
    $this->initIntArray10();
    $this->initStringArray10();
    $this->initObjectArray10();
    $this->initIntArray100();
    $this->initStringArray100();
    $this->initObjectArray100();
  }

  private function initIntArray10() {
    for ($iter = 0; $iter < 10; $iter++ ) {
      $this->intArray10[$iter] = 0;
    }
  }

  private function initStringArray10() {
    for ($iter = 0; $iter < 10; $iter++ ) {
      $this->stringArray10[$iter] = "0";
    }
  }

  private function initObjectArray10() {
    for ($iter = 0; $iter < 10; $iter++ ) {
      $this->objectArray10[$iter] = new ItTestClassArray() ;
    }
  }

  private function initIntArray100() {
    for ($iter = 0; $iter < 100; $iter++ ) {
      $this->intArray100[$iter] = 0;
    }
  }

  private function initStringArray100() {
    for ($iter = 0; $iter < 100; $iter++ ) {
      $this->stringArray100[$iter] = "0";
    }
  }

  private function initObjectArray100() {
    for ($iter = 0; $iter < 100; $iter++ ) {
      $this->objectArray100[$iter] = new ItTestClassArray();
    }
  }

  public function sumNum20() {
    $result = 0;
    for ($iter = 1; $iter <= 20; $iter++) {
      $result += $iter;
    }
    return ($result);
  }

  public function sumNum100() {
    $result = 0;
    for ($iter = 1; $iter <= 100; $iter++) {
      $result += $iter;
    }
    return ($result);
  }

  public function iterateIntArray10() {
    foreach ($this->intArray10 as $elem) {
      $elem = 1;
    }
  }

  public function iterateIntArrayFor10() {
    for ($iter = 0; $iter < 10; $iter++) {
      $this->intArray10[$iter] = 1;
    }
  }

  public function iterateStringArray10() {
    foreach ($this->stringArray10 as $elem) {
      $elem = "1";
    }
  }

  public function iterateStringArrayFor10() {
    for ($iter = 0; $iter < 10; $iter++) {
      $this->stringArray10[$iter] = "1";
    }
  }

  public function iterateObjectArray10() {
    foreach ($this->objectArray10 as $elem) {
      $elem = new ItTestClassArray();
    }
  }

  public function iterateObjectArrayFor10() {
    for ($iter = 0; $iter < 10; $iter++) {
      $this->objectArray10[$iter] = new ItTestClassArray();
    }
  }

  public function iterateIntArray100() {
    foreach ($this->intArray100 as $elem) {
      $elem = 2;
    }
  }

  public function iterateIntArrayFor100() {
    for ($iter = 0; $iter < 100; $iter++) {
      $this->intArray10[$iter] = 2;
    }
  }

  public function iterateStringArray100() {
    foreach ($this->stringArray100 as $elem) {
      $elem = "2";
    }
  }

  public function iterateStringArrayFor100() {
    for ($iter = 0; $iter < 100; $iter++) {
      $this->stringArray100[$iter] = "2";
    }
  }

  public function iterateObjectArray100() {
    foreach ($this->objectArray100 as $elem) {
      $elem = new ItTestClassArray();
    }
  }

  public function iterateObjectArrayFor100() {
    for ($iter = 0; $iter < 100; $iter++) {
      $this->objectArray100[$iter] = new ItTestClassArray();
    }
  }
 }

class ItTestClassVector {
}

class IteratorTestVector {
  private $intVector10;
  private $stringVector10;
  private $objectVector10;
  private $intVector100;
  private $stringVector100;
  private $objectVector100;

  public function __construct() {
    $this->initIntVector10();
    $this->initStringVector10();
    $this->initObjectVector10();
    $this->initIntVector100();
    $this->initStringVector100();
    $this->initObjectVector100();
  }

  private function initIntVector10() {
    for ($iter = 0; $iter < 10; $iter++ ) {
      $this->intVector10[$iter] = 0 ;
    }
  }

  private function initStringVector10() {
    for ($iter = 0; $iter < 10; $iter++ ) {
      $this->stringVector10[$iter] = "0" ;
    }
  }

  private function initObjectVector10() {
    for ($iter = 0; $iter < 10; $iter++ ) {
      $this->objectVector10[$iter] = new ItTestClassVector() ;
    }
  }

  private function initIntVector100() {
    for ($iter = 0; $iter < 100; $iter++ ) {
      $this->intVector100[$iter] = 0;
    }
  }

  private function initStringVector100() {
    for ($iter = 0; $iter < 100; $iter++ ) {
      $this->stringVector100[$iter] = "0";
    }
  }

  private function initObjectVector100() {
    for ($iter = 0; $iter < 100; $iter++ ) {
      $this->objectVector100[$iter] = new ItTestClassVector();
    }
  }

  public function iterateIntVector10() {
    foreach ($this->intVector10 as $elem) {
      $elem = 1;
    }
  }

  public function iterateIntVectorFor10() {
    for ($iter = 0; $iter < 10; $iter++) {
      $this->intVector10[$iter] = 1;
    }
  }

  public function iterateStringVector10() {
    foreach ($this->stringVector10 as $elem) {
      $elem = "1";
    }
  }

  public function iterateStringVectorFor10() {
    for ($iter = 0; $iter < 10; $iter++) {
      $this->stringVector10[$iter] = "1";
    }
  }

  public function iterateObjectVector10() {
    foreach ($this->objectVector10 as $elem) {
      $elem = new ItTestClassVector();
    }
  }

  public function iterateObjectVectorFor10() {
    for ($iter = 0; $iter < 10; $iter++) {
      $this->objectVector10[$iter] = new ItTestClassVector();
    }
  }

  public function iterateIntVector100() {
    foreach ($this->intVector100 as $elem) {
      $elem = 2;
    }
  }

  public function iterateIntVectorFor100() {
    for ($iter = 0; $iter < 100; $iter++) {
      $this->intVector10[$iter] = 2;
    }
  }

  public function iterateStringVector100() {
    foreach ($this->stringVector100 as $elem) {
      $elem = "2";
    }
  }

  public function iterateStringVectorFor100() {
    for ($iter = 0; $iter < 100; $iter++) {
      $this->stringVector100[$iter] = "2";
    }
  }

  public function iterateObjectVector100() {
    foreach ($this->objectVector100 as $elem) {
      $elem = new ItTestClassVector();
    }
  }

  public function iterateObjectVectorFor100() {
    for ($iter = 0; $iter < 100; $iter++) {
      $this->objectVector100[$iter] = new ItTestClassVector();
    }
  }
 }

class RegEx {
  public function regexPCRELen18() {
    $str = "  this is the hhvm";
    return (preg_match ("/this\s*(\S*)/", $str));
  }

  public function regexPOSIXLen18() {
    $str = "  this is the hhvm";
    return (ereg ("this\s*(\S*)", $str));
  }

  public function regexByHandLen18() {
    $str = "  this is the hhvm";
    if (strstr ($str, "this ")) {
      return (1);
    }
    else {
      return (0);
    }
  }

  public function regexPCRELen85() {
    $str = <<<EOD
    this is the hhvm
    0123456789
    0123456789
    0123456789
    0123456789
    0123456789
    0123456789
    012345678
EOD;
    return (preg_match ("/this\s*(\S*)/", $str));
  }

  public function regexPOSIXLen85() {
    $str = <<<EOD
    this is the hhvm
    0123456789
    0123456789
    0123456789
    0123456789
    0123456789
    0123456789
    012345678
EOD;
    return (ereg ("this\s*(\S*)", $str));
  }

  public function regexByHandLen85() {
    $str = <<<EOD
    this is the hhvm
    0123456789
    0123456789
    0123456789
    0123456789
    0123456789
    0123456789
    012345678
EOD;
    if (strstr ($str, "this ")) {
      return (1);
    }
    else {
      return (0);
    }
  }

  public function regexPCRELen152() {
     $str = <<<EOD
     this is the hhvm
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     012345
EOD;
    return (preg_match ("/this\s*(\S*)/", $str));
  }

  public function regexPOSIXLen152() {
   $str = <<<EOD
     this is the hhvm
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     012345
EOD;
    return (ereg ("this\s*(\S*)", $str));
  }

  public function regexByHandLen152() {
  $str = <<<EOD
     this is the hhvm
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     0123456789
     012345
EOD;
    if (strstr ($str, "this ")) {
      return (1);
    }
    else {
      return (0);
    }
  }
}

class ReflectionTestClass {
  public $testID = 1;

  public function getID() {
    return $this->testID;
  }

  public function setID($value) {
    $this->testID = $value;
  }

  public static function emptyStatic() {
    return 1;
  }

  public static function emptyStatic5Arg (
    $one, $two, $three, $four, $five) {
    return 1;
  }

}

class ReflectionTest {
  public function reflectionGetObjectClassName() {
    $object = new ReflectionTestClass();
    $reflectionobj = new ReflectionObject($object);
    if ($reflectionobj->name == "ReflectionTestClass") {
      return 1;
    }
    else {
      return 0;
    }
  }

  public function regularGetType() {
    $object = new ReflectionTestClass();
    if (gettype($object) == "object") {
      return 1;
    }
    else {
      return 0;
    }
   }

  public function reflectionInvokeEmptyStatic() {
    $reflectionmethod = new ReflectionMethod(
      'ReflectionTestClass::emptyStatic');
    return ($reflectionmethod->invoke(null));
  }

  public function reflectionInvokeEmptyStatic5Arg() {
    $reflectionmethod = new ReflectionMethod(
      'ReflectionTestClass::emptyStatic5Arg');
    return ($reflectionmethod->invoke(null, 1, 2, 3, 4, 5));
  }

  public function reflectionInvokeArgsEmptyStatic5Arg() {
    $reflectionmethod = new ReflectionMethod(
      'ReflectionTestClass::emptyStatic5Arg');
    return ($reflectionmethod->invokeArgs(null, array(1, 2, 3, 4, 5)));
  }

  public function regularEmptyStaticMethodCall() {
    return (ReflectionTestClass::emptyStatic());
  }

  public function regularEmptyStaticMethod5ArgCall() {
    return (ReflectionTestClass::emptyStatic5Arg(1, 2, 3, 4, 5));
  }

  public function reflectionGetInstanceIntProp() {
    $reflectionclass = new ReflectionClass('ReflectionTestClass');
    $object = new ReflectionTestClass();
    return ($reflectionclass->getProperty('testID')->getValue($object));
  }

  public function reflectionSetInstanceIntProp() {
    $reflectionclass = new ReflectionClass('ReflectionTestClass');
    $object = new ReflectionTestClass();
    $reflectionclass->getProperty('testID')->setValue($object, 2);
    return 1;
  }

  public function regularSetInstanceIntProp() {
    $object = new ReflectionTestClass();
    $object->setID(2);
    return 1;
  }
}
