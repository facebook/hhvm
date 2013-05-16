<?php
/* Prototype: string php_uname  ([ string $mode  ] )
 * Description:  Returns information about the operating system PHP is running on
*/

echo "*** Testing php_uname() - error test\n";

echo "\n-- Testing php_uname() function with more than expected no. of arguments --\n";
var_dump( php_uname('a', true) );

echo "\n-- Testing php_uname() function with invalid mode --\n";
// am invalid mode shoudl result in same o/p as mode 'a'
var_dump( php_uname('z') == php_uname('z') ); 

class barClass {
}

$fp = fopen(__FILE__, "r");

echo "\n-- Testing php_uname() function with invalid argument types --\n";
var_dump(php_uname(array()));
var_dump(php_uname(array('color' => 'red', 'item' => 'pen')));
var_dump(php_uname(new barClass()));
var_dump(php_uname($fp));

fclose($fp);
?>
===DONE===