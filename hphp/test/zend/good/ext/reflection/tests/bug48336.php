<?php
class A {
}

class B extends A {
  static protected $prop;
}

class C extends B {
  static protected $prop;
}

class D extends C {
}

class E extends D {
}

class F extends E {
  static protected $prop;
}

$class = 'A';
for($class = 'A'; $class <= 'F'; $class ++) {
  print($class.' => ');
  try {
    $rp = new ReflectionProperty($class, 'prop');
    print($rp->getDeclaringClass()->getName());
  } catch(Exception $e) {
    print('N/A');
  }
  print("\n");
}
?>