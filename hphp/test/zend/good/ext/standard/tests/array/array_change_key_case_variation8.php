<?hh
/* Prototype  : array array_change_key_case(array $input [, int $case])
 * Description: Retuns an array with all string keys lowercased [or uppercased]
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_change_key_case() behaves with different strings
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_change_key_case() : usage variations ***\n";

$inputs = varray [
    // group of escape sequences
    dict['' => 1, '' => 2, "\a" => 3, "\cx" => 4, "\e" => 5, "\f" => 6, "\n" => 7, "\t" => 8, "\xhh" => 9, "\ddd" => 10, "\v" => 11],

    // array contains combination of capital/small letters
    dict["lemoN" => 1, "Orange" => 2, "banana" => 3, "apple" => 4, "Test" => 5, "TTTT" => 6, "ttt" => 7, "ww" => 8, "x" => 9, "X" => 10, "oraNGe" => 11, "BANANA" => 12]
];

foreach($inputs as $input) {
    echo "\n-- \$case = default --\n";
    var_dump(array_change_key_case($input));
    echo "-- \$case = upper --\n";
    var_dump(array_change_key_case($input, CASE_UPPER));
}

echo "Done";
}
