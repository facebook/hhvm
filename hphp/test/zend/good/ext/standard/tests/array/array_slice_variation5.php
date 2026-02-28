<?hh
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length
 * Source code: ext/standard/array.c
 */

/*
 * Pass different integers as $offset argument to test how array_slice() behaves
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_slice() : usage variations ***\n";

$input = dict['one' => 1, 2 => 'two', 3 => 'three', 9 => 'nine', 'ten' => 10];

for ($i = -7; $i <= 7; $i++) {
    echo "\n-- \$offset is $i --\n";
    var_dump(array_slice($input, $i));
}
echo "\n-- \$offset is maximum integer value --\n";
var_dump(array_slice($input, PHP_INT_MAX));

echo "\n-- \$offset is minimum integer value --\n";
var_dump(array_slice($input, -PHP_INT_MAX));

echo "Done";
}
