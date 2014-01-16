<?php

function outputPrototype($rf) {
  try {
    $prototype = $rf->getPrototype();
    print $prototype->getDeclaringClass()->getName() .
      "::" . $prototype->getName();
    print "\n";
  } catch (ReflectionException $re) {
    print $re->getMessage(). "\n";
  }
}

interface Int1 {
  function method1();
}
interface Int2 {
  function method2();
}
interface Int3 extends Int2 {
  function method3();
}
interface Int4 {
  function method4();
}

class Cls1 {
  public function method1() {
  }
  private function method2() {
  }
}
class Cls2 {
  public function method1() {
  }
  public function method2() {
  }
}
abstract class Cls3 extends Cls2 implements Int3 {
}
class Cls4 extends Cls3 {
  public function method3() {
  }
}
class Cls5 extends Cls4 implements Int4 {
  public function method4() {
  }
}

trait Method3 {
  public function method3() {
  }
}

class Cls6 extends Cls4 implements Int4 {
  use Method3;
  function method4() {
  }
}
class Cls7 extends Cls6 {
  public function method1() {
  }
  public function method7() {
  }
}
class Cls8 extends Cls7 {
  public function method7() {
  }
}

class PDOSubClass extends PDO {
  public function commit() {
    return true;
  }
}

function main() {
  $tests = array(
    array("Cls1", "method1"),
    array("Cls1", "method2"),
    array("Cls2", "method2"),
    array("Cls3", "method1"),
    array("Cls3", "method3"),
    array("Cls4", "method1"),
    array("Cls4", "method3"),
    array("Cls5", "method3"),
    array("Cls5", "method4"),
    array("Cls6", "method3"),
    array("Cls6", "method4"),
    array("Cls6", "method1"),
    array("Cls7", "method1"),
    array("Cls7", "method7"),
    array("Cls8", "method1"),
    array("Cls8", "method7"),
    // builtin class
    array("PDO", "commit"),
    // sublcass of builtin class
    array("PDOSubClass", "commit"),
  );

  foreach ($tests as $test) {
    $rf = new ReflectionMethod($test[0], $test[1]);
    outputPrototype($rf);
  }
}

main();

