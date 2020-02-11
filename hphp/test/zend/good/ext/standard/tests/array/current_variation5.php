<?hh
/* Prototype  : mixed current(array $array_arg)
 * Description: Return the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 * Alias to functions: pos
 */

function current_variation5_ref(inout $a)
{
    var_dump(current($a));
    var_dump(next(inout $a));
}

function current_variation5($a)
{
    var_dump(current($a));
    var_dump(next(inout $a));
}
<<__EntryPoint>> function main(): void {
echo "*** Testing current() : usage variations ***\n";

echo "\n-- Function: reference parameter --\n";

$a = varray['yes', 'maybe', 'no'];

var_dump(current($a));
var_dump(next(inout $a));
current_variation5($a);

echo "\n-- Function: normal parameter --\n";

$a = varray['yes', 'maybe', 'no'];

var_dump(current($a));
var_dump(next(inout $a));
current_variation5($a);

echo "===DONE===\n";
}
