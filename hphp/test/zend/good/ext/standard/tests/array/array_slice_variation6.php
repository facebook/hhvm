<?hh
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length
 * Source code: ext/standard/array.c
 */

/*
 * Pass different integer values as $length argument to array_slice() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_slice() : usage variations ***\n";

$input = dict['one' => 1, 2 => 'two', 3 => 'three', 9 => 'nine', 'ten' => 10];
$offset = 1;

for ($i = -6; $i <= 6; $i++) {
    echo "\n-- \$length is $i --\n";
    var_dump(array_slice($input, $offset, $i));
}
echo "\n-- \$length is maximum integer value --\n";
var_dump(array_slice($input, $offset, PHP_INT_MAX));

echo "\n-- \$length is minimum integer value --\n";
var_dump(array_slice($input, $offset, -PHP_INT_MAX));

echo "Done";
}
