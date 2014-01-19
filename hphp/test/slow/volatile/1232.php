<?php

spl_autoload_register(function($f) {
 var_dump(1);
 }
);
spl_autoload_register(function($f) {
 var_dump(2);
 }
);
class_exists('A');
// hphpc won't call the autoloader unless there exists a
// definition for the class somewhere
if (true) {
  class A {
}
}
