<?hh
/* Prototype  : bool ctype_punct(mixed $c)
 * Description: Checks for any printable character which is not whitespace
 * or an alphanumeric character
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different octal and hexadecimal values to ctype_punct() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_punct() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

$octal_values = vec[041,  042,  043,  044];
$hex_values   = vec[0x21, 0x22, 0x23, 0x24];

echo "\n-- Octal Values --\n";
$iterator = 1;
foreach($octal_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_punct($c));
    $iterator++;
}

echo "\n-- Hexadecimal Values --\n";
$iterator = 1;
foreach($hex_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_punct($c));
    $iterator++;
}

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
