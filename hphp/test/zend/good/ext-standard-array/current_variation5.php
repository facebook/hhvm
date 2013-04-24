<?php
/* Prototype  : mixed current(array $array_arg)
 * Description: Return the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 * Alias to functions: pos
 */

echo "*** Testing current() : usage variations ***\n";

echo "\n-- Function: reference parameter --\n";

function current_variation5_ref(&$a)
{
    var_dump(current($a));
    var_dump(next($a));
}

$a = array('yes', 'maybe', 'no');

var_dump(current($a));
var_dump(next($a));
current_variation5($a);

echo "\n-- Function: normal parameter --\n";

function current_variation5($a)
{
    var_dump(current($a));
    var_dump(next($a));
}

$a = array('yes', 'maybe', 'no');

var_dump(current($a));
var_dump(next($a));
current_variation5($a);

?>
===DONE===
<?php exit(0); ?>