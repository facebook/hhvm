<?php

function get_declared_user_traits() {
  $ret = array();
  foreach (get_declared_traits() as $v) {
    // exclude system traits
    $rc = new ReflectionClass($v);
    if ('systemlib.php' !== basename($rc->getFileName())) {
      $ret[] = $v;
    }
  }
  return $ret;
}
/* Prototype  : proto array get_declared_traits()
 * Description: Returns an array of all declared traits.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */


echo "*** Testing get_declared_traits() : basic functionality ***\n";

trait MyTrait {
}

// Zero arguments
echo "\n-- Testing get_declared_traits() function with Zero arguments --\n";
var_dump(get_declared_user_traits());

foreach (get_declared_user_traits() as $trait) {
  if (!trait_exists($trait)) {
    echo "Error: $trait is not a valid trait.\n";
  }
}

echo "\n-- Ensure trait is listed --\n";
var_dump(in_array('MyTrait', get_declared_user_traits()));

echo "\n-- Ensure userspace interfaces are not listed --\n";
interface I {
}
var_dump(in_array( 'I', get_declared_user_traits()));

echo "\n-- Ensure userspace classes are not listed --\n";
class MyClass {
}
var_dump(in_array( 'MyClass', get_declared_user_traits()));


echo "Done\n";
?>
