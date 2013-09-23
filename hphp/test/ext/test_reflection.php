<?php

interface IFoo {

  function foo();
  function bar();
}

abstract class Foo implements IFoo {
}

abstract class Bar extends Foo {
  function foo() {
    print "foo";
  }
  function foobar() {
    print "foobar";
  }
}

class Baz extends Bar {
  function bar() {
    print "bar";
  }
  function baz() {
    print "baz";
  }
}

interface IFoo2 {

  function foo2();
  function bar2();
}

abstract class Foo2 implements IFoo2 {
}

abstract class Bar2 extends Foo2 {
  function foo2() {
    print "foo2";
  }
  function foobar2() {
    print "foobaar2";
  }
}

abstract class Baz2 extends Bar2 {
  function bar2() {
    print "bar2";
  }
  function baz2() {
    print "baz2";
  }
}

function test_class_reflector() {
  $ifoo_reflector = new ReflectionClass("IFoo");
  foreach ($ifoo_reflector->getMethods() as $method) {
    var_dump($method->getName());
    var_dump($method->isAbstract());
  }
  $foo_reflector = new ReflectionClass("Foo");
  foreach ($foo_reflector->getMethods() as $method) {
    var_dump($method->getName());
    var_dump($method->isAbstract());
  }
  $baz_reflector = new ReflectionClass("Baz");
  foreach ($baz_reflector->getMethods() as $method) {
    var_dump($method->getName());
    var_dump($method->isAbstract());
  }
}

function test_method_reflector() {
  $ifoo2__foo2_reflector = new ReflectionMethod("IFoo2::foo2");
  var_dump($ifoo2__foo2_reflector->getName());
  var_dump($ifoo2__foo2_reflector->isAbstract());
  $foo2__foo2_reflector = new ReflectionMethod("Foo2::foo2");
  var_dump($foo2__foo2_reflector->getName());
  var_dump($foo2__foo2_reflector->isAbstract());
  $bar2__foo2_reflector = new ReflectionMethod("Bar2::foo2");
  var_dump($bar2__foo2_reflector->getName());
  var_dump($bar2__foo2_reflector->isAbstract());
  $bar2__bar2_reflector = new ReflectionMethod("Bar2::bar2");
  var_dump($bar2__bar2_reflector->getName());
  var_dump($bar2__bar2_reflector->isAbstract());
  $bar2__foo2_reflector = new ReflectionMethod("Bar2::foo2");
  $bar2__foobar2_reflector = new ReflectionMethod("Bar2::foobar2");
  var_dump($bar2__foobar2_reflector->getName());
  var_dump($bar2__foobar2_reflector->isAbstract());
  $baz2__foo2_reflector = new ReflectionMethod("Bar2::foo2");
  var_dump($baz2__foo2_reflector->getName());
  var_dump($baz2__foo2_reflector->isAbstract());
  $baz2__foobar2_reflector = new ReflectionMethod("Baz2::foobar2");
  var_dump($baz2__foobar2_reflector->getName());
  var_dump($baz2__foobar2_reflector->isAbstract());
}

test_class_reflector();
test_method_reflector();

