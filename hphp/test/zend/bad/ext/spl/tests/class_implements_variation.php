<?php
/* Prototype  : array class_implements(mixed what [, bool autoload ])
 * Description: Return all classes and interfaces implemented by SPL 
 * Source code: ext/spl/php_spl.c
 * Alias to functions: 
 */

echo "*** Testing class_implements() : variation ***\n";

echo "--- testing no interfaces ---\n";
class fs {}
var_dump(class_implements(new fs));
var_dump(class_implements('fs'));

echo "\n--- testing autoload ---\n";
var_dump(class_implements('non_existent'));
var_dump(class_implements('non_existent2', false));


function __autoload($classname) {
   echo "attempting to autoload $classname\n";
}

?>
===DONE===