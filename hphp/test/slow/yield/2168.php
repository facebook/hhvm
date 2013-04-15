<?php

 
function f() {
  $var = hphp_get_iterator(null);
  var_dump(is_null($var));
  var_dump(is_object($var));
  var_dump(get_class($var));
}
f();
