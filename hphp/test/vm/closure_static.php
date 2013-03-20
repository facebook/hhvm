<?php

class A {
  private $c = 1;

  function b() {
    $a = function () {
      var_dump($this);
    };
    $a();

    $a = static function () {
      var_dump($this);
    };
    $a();
  }

  static function c() {
    $a = function () {
      var_dump($this);
    };
    $a();

    $a = static function () {
      var_dump($this);
    };
    $a();
  }
}

(new A)->b();
(new A)->c();
