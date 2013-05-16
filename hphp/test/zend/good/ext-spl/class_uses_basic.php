<?php
/* Prototype  : array class_uses(mixed what [, bool autoload ])
 * Description: Return all traits used by a class
 * Source code: ext/spl/php_spl.c
 * Alias to functions: 
 */

echo "*** Testing class_uses() : basic ***\n";


trait foo { }
class bar { use foo; }

var_dump(class_uses(new bar));
var_dump(class_uses('bar'));


?>
===DONE===