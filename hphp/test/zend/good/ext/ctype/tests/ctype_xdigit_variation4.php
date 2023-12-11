<?hh
/* Prototype  : bool ctype_xdigit(mixed $c)
 * Description: Checks for character(s) representing a hexadecimal digit
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different hexadecimal and octal values that:
 * 1. contain hexadecimal digits
 * 2. correspond to character codes recognised as hexadecimal digits (see variation2)
 *    referred to as 'correct' integers below
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_xdigit() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

// contain hexadecimal digits but do not correspond to 'correct' ints
$octal_values1 = vec[012, 013, 014, 015];

// correspond to 'correct' integers
$octal_values2 = vec[061, 062, 063, 064];

// contain hexadecimal digits but do not correspond to 'correct' ints
$hex_values1 = vec[0x1A, 0x1B, 0x1C, 0x1D];

//correspond to 'correct' integers
$hex_values2 = vec[0x61, 0x62, 0x63, 0x64];

echo "\n-- Octal values --\n";
echo "'Incorrect' Integers: \n";
foreach($octal_values1 as $c) {
    var_dump(ctype_xdigit($c));
}
echo "'Correct' Integers: \n";
foreach($octal_values2 as $c) {
    var_dump(ctype_xdigit($c));
}

echo "\n-- Hexadecimal values --\n";
echo "'Incorrect' Integers: \n";
foreach($hex_values1 as $c) {
    var_dump(ctype_xdigit($c));
}
echo "'Correct' Integers: \n";
foreach($hex_values2 as $c) {
    var_dump(ctype_xdigit($c));
}
setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
