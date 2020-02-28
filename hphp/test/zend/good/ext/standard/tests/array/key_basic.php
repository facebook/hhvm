<?hh
/* Prototype  : mixed key(array $array_arg)
 * Description: Return the key of the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of key()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing key() : basic functionality ***\n";

$array = darray[0 => 'zero', 99 => 'one', 100 => 'two', 'three' => 3];
echo "\n-- Initial Position: --\n";
var_dump(key($array));

echo "\n-- Next Position: --\n";
next(inout $array);
var_dump(key($array));

echo "\n-- End Position: --\n";
end(inout $array);
var_dump(key($array));

echo "\n-- Past end of the array --\n";
next(inout $array);
var_dump(key($array));
echo "===DONE===\n";
}
