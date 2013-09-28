<?php

function autoload_first($name) {
  echo __METHOD__ . "\n";
}
function autoload_second($name) {
  echo __METHOD__ . "\n";
}
function __autoload($name) {
  echo __METHOD__ . "\n";
}
echo "**************\n";
class_exists('A');
echo "**************\n";
spl_autoload_register('autoload_first');
spl_autoload_register('autoload_second');
class_exists('B');
echo "**************\n";
spl_autoload_unregister('autoload_first');
spl_autoload_unregister('autoload_second');
class_exists('C');
echo "**************\n";
spl_autoload_unregister('spl_autoload_call');
class_exists('D');
echo "**************\n";
// hphpc won't call the autoloader unless there exists a 
// definition for the class somewhere
if (true) {
  class A {
}
  class B {
}
  class C {
}
  class D {
}
}
