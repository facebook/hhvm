<?php

if (true) {
  class A {
    var $a0;
    static $a1 = 1;
    static $a2 = 2;
  }
  class X {
    var $x0;
    static $x1 = 1;
    static $x2 = 2;
  }
}
 else {
  class A {
    var $a3;
    static $a4 = 4;
    static $a5 = 5;
  }
  class X {
    var $y3;
    static $y4 = 4;
    static $y5 = 5;
  }
}
class B extends A {
  var $b0 = 3;
  static $b1 = 4;
  static $b2 = 5;
}
class Y extends X {
  var $y0 = 3;
  static $y1 = 4;
  static $y2 = 5;
}
class C {
  var $c0;
  static $c1 = 1;
  static $c2 = 2;
}
class Z {
  var $z0;
  static $z1 = 1;
  static $z2 = 2;
}
$vars = get_class_vars('A');
 asort($vars);
 var_dump($vars);
$vars = get_class_vars('B');
 asort($vars);
 var_dump($vars);
$vars = get_class_vars('C');
 asort($vars);
 var_dump($vars);
$vars = get_class_vars('X');
 asort($vars);
 var_dump($vars);
$vars = get_class_vars('Y');
 asort($vars);
 var_dump($vars);
$vars = get_class_vars('Z');
 asort($vars);
 var_dump($vars);
