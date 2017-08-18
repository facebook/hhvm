<?php

class A {
  function __construct() {
    echo "A\n";
  }
}

class B extends A { }

class C extends A {
  function __construct() {
    parent::__construct();
    echo "C\n";
  }
}

new A();
new B();
new C();
