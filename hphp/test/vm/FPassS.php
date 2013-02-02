<?php

print "Test begin\n";

class NewClass {
  static public $a;
}

NewClass::$a = array(1,2,3);
while (($b = array_pop(NewClass::$a))) {
  echo $b . "\n";
  var_dump(NewClass::$a);
}

print "Test end\n";
