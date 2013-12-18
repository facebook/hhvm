<?php

class Hello {
  public function sayHello($name) {
    return "Hello $name!";
  }
}

class StackTest extends PHPUnit_Framework_TestCase {
  public function testTrueIsActuallyTrue() {
    $stub = $this->getMock('Hello');
    $this->assertTrue(true);
  }
}
