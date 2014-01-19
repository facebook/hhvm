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
$traits = get_declared_user_traits();
var_dump($traits);
var_dump(in_array('T1', $traits));
var_dump(in_array('T1', get_declared_user_traits()));
