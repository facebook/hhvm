<?hh
/* Prototype  : bool ctype_digit(mixed $c)
 * Description: Checks for numeric character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass octal and hexadecimal values as $c argument to ctype_digit() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_digit() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

$octal_values = vec[061,  062,  063,  064];
$hex_values = vec  [0x31, 0x32, 0x33, 0x34];

echo "\n-- Octal Values --\n";
$iterator = 1;
foreach($octal_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_digit($c));
    $iterator++;
}

echo "\n-- Hexadecimal Values --\n";
$iterator = 1;
foreach($hex_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_digit($c));
    $iterator++;
}

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
