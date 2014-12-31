<?php

#===============================================================================
# ReflectionFunction.

class MyReflectionFunction extends ReflectionFunction
{
  public function __get($name)
  {
    return 'undefined';
  }

  public function __set($name, $value)
  {
    $this->{$name} = $value . '_bar';
  }
}

function f() {
}

$reflection = new MyReflectionFunction('f');
$reflection->testProperty = 'foo';

var_dump($reflection->name);
var_dump($reflection->testProperty);
var_dump($reflection->undefinedProperty);


#===============================================================================
# ReflectionClass.

class MyReflectionClass extends ReflectionClass
{
  public function __get($name)
  {
    return 'undefined';
  }

  public function __set($name, $value)
  {
    $this->{$name} = $value . '_bar';
  }
}

class TestClass{
}

$reflection = new MyReflectionClass('TestClass');
$reflection->testProperty = 'foo';

var_dump($reflection->name);
var_dump($reflection->testProperty);
var_dump($reflection->undefinedProperty);


#===============================================================================
# ReflectionMehthod.

class MyReflectionMethod extends ReflectionMethod
{
  public function __get($name)
  {
    return 'undefined';
  }

  public function __set($name, $value)
  {
    $this->{$name} = $value . '_bar';
  }
}

class TestClassWithMethod{
  public function m() {
  }
}

$reflection = new MyReflectionMethod('TestClassWithMethod::m');
$reflection->testProperty = 'foo';

var_dump($reflection->name);
var_dump($reflection->class);
var_dump($reflection->testProperty);
var_dump($reflection->undefinedProperty);
