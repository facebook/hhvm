<?php

$foo = function() {
  static $a;
  $a++;
  print "foo $a\n";
};

$bar = function() {
  static $a;
  $a += 2;
  print "bar $a\n";
};

$init_twice = function($x) {
  if (!$x) {
    static $a = 42;
    $a--;
    print "init0 $a\n";
  } else {
    static $a = 47;
    $a++;
    print "init1 $a\n";
  }
};

class A {
  private static $priv;
  public $baz;

  public function __construct() {
    if (self::$priv === null) {
      self::$priv = function() {
        static $x = 0;
        $x += 4;
        print "A::priv $x\n";
      };
    }

    $this->baz = function() {
      static $a;
      $a += 4;
      print "A::baz $a\n";
      $priv = self::$priv;
      $priv();
    };
  }
}

class B extends A {
}

$a = new A();
$b = new B();
$abaz = $a->baz;
$bbaz = $b->baz;

$foo();
$bar();
$abaz();
$bbaz();
$foo();
$bar();
$abaz();
$bbaz();
$init_twice(0);
$init_twice(0);
$init_twice(0);
$init_twice(1);
$init_twice(1);
$init_twice(1);
