<?php

class A {
  function A() { echo "I'm A"; }
}

class B extends A {
  function __construct() {
    // Incorrect capitalization is intentional
    parent::__constrUct();
  }
}

$b = new B;
