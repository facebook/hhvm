<?php

class MyDestructableClass {
   function __construct() {
       print "In constructor\n";
       $this->name = "MyDestructableClass";
   }
   function __destruct() {
       print "Destroying " . $this->name . "\n";
   }
}
function foo($a) {
  if ($a) return new MyDestructableClass();
  return false;
}
function bar($a) {
  if ($a) {
    $obj = foo(1);
    $obj = 1;
    var_dump(2);
  }
  var_dump(1);
}
bar(1);
