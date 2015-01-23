<?php

trait T1 {
  public function t1method() {}
  public function t2method() {}
  public function t3method() {}
}

trait T2 {
  public function bmethod() {}
}

class A1 {
  use T1 {
    t2method as a1method;
    t2method as a3method;
    t2method as bmethod;
    t2method as a2method;
  }
}

class A2 {
  use T2;
}

class B1 extends A1 {
  public function bmethod() {}
}

class B2 extends A1 {
  use T2;

  public function zmethod() {}
}

class B3 extends A2 {
  use T1 {
    t2method as a1method;
  }

  public function zmethod() {}
}

class B4 extends A2 {
  use T1 {
    t2method as a1method;
    t2method as a3method;
    t2method as bmethod;
    t2method as a2method;
  }

  public function zmethod() {}
}

foreach (array('B1', 'B2', 'B3', 'B4') as $name) {
  $obj = new ReflectionClass($name);
  var_dump(array_map(
    function($meth) {
      return $meth->getDeclaringClass()->getName().'::'.$meth->getName();
    },
    $obj->getMethods()
  ));
}
