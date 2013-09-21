<?php
/* Prototype  : proto string ob_get_contents(void)
 * Description: Return the contents of the output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */


echo "*** Testing ob_get_contents() : error cases ***\n";

var_dump(ob_get_contents("bob"));

ob_start();

var_dump(ob_get_contents("bob2",345));

echo "Done\n";
?>