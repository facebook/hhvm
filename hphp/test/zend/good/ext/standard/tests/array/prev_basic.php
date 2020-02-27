<?hh
/* Prototype  : mixed prev(array $array_arg)
 * Description: Move array argument's internal pointer to the previous element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of prev()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing prev() : basic functionality ***\n";

$array = varray['zero', 'one', 'two'];
end(inout $array);
echo key($array) . " => " . current($array) . "\n";
var_dump(prev(inout $array));

echo key($array) . " => " . current($array) . "\n";
var_dump(prev(inout $array));

echo key($array) . " => " . current($array) . "\n";
var_dump(prev(inout $array));

echo "\n*** Testing an array with differing values/keys ***\n";
$array2 = darray[0 => 'one', 2 => "help", 3 => 3, 4 => false, 'stringkey2' => 'val2', 'stringkey1' => 'val1'];
end(inout $array2);
$length = count($array2);
for ($i = $length; $i > 0; $i--) {
    var_dump(prev(inout $array2));
}

echo "===DONE===\n";
}
