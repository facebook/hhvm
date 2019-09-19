<?hh
/* Prototype  : mixed current(array $array_arg)
 * Description: Return the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of current()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing current() : basic functionality ***\n";

$array = array ('zero', 'one', 'two', 'three' => 3);
var_dump(current($array));
next(inout $array);
var_dump(current($array));
end(inout $array);
var_dump(current($array));
next(inout $array);
var_dump(current($array));
echo "===DONE===\n";
}
