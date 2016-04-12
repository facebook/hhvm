<?php

trait Foo {
  public static function f() {
    echo __METHOD__."\n";
    echo __TRAIT__."\n";
    $a = new class {
      public static function f() {
        echo __METHOD__."\n";
        echo __TRAIT__."\n";
      }
    };
    $a::f();
    echo __METHOD__."\n";
    echo __TRAIT__."\n";
  }
}

Foo::f();
