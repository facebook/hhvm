<?php

/**
 * This is more of a test for testing the behavior of
 * serialization/unserialization when a class with native data implements
 * __sleep() and __wakeup() methods.
 */

class Foo {
}

class Bar extends ReflectionClass {

  public $prop = 123;
  public $dontSerializeMe = "not serialized";
  private $meh = 456;

  public function __sleep() {
    var_dump("__sleep invoked");
    var_dump($this->name);
    var_dump($this->getName());
    return array('prop', 'meh');
  }

  public function __wakeup() {
    var_dump("__wakeup invoked");
    var_dump($this->prop);
    var_dump($this->dontSerializeMe);
    var_dump($this->name);
    var_dump($this->getName());
  }

}

$rc = new Bar(Foo::class);
$rc->prop = 1337;
$rc->dontSerializeMe = "serialized";
$serialized = serialize($rc);
var_dump(json_encode($serialized));
var_dump(unserialize($serialized));
