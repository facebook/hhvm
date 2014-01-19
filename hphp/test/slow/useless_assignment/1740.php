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
function foo(&$a) {
  $a = new MyDestructableClass();
}
function bar($a) {
  if ($a) {
    $b = array(1, 2, 3);
    var_dump($b);
    foo($dummy = array(1, 2, 3));
    $c = array(1, 2, 3);
    var_dump($c);
  }
}
bar(1);
