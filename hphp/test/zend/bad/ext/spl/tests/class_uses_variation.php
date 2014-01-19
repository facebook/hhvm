<?php
/* Prototype  : array class_uses(mixed what [, bool autoload ])
 * Description: Return all traits used by a class
 * Source code: ext/spl/php_spl.c
 * Alias to functions: 
 */

echo "*** Testing class_uses() : variation ***\n";

echo "--- testing no traits ---\n";
class fs {}
var_dump(class_uses(new fs));
var_dump(class_uses('fs'));

echo "\n--- testing autoload ---\n";
var_dump(class_uses('non_existent'));
var_dump(class_uses('non_existent2', false));


function __autoload($classname) {
   echo "attempting to autoload $classname\n";
}

?>
===DONE===