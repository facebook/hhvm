<?php
class foo {

  public function __construct() {
    print "this should only be printed three times!\n";
  }

}

$c1 = new foo();
$c2 = (new ReflectionClass('foo'))->newInstance();
$c3 = (new ReflectionClass('foo'))->newInstanceArgs(array());
$no_c = (new ReflectionClass('foo'))->newInstanceWithoutConstructor();
