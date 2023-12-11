<?hh
/* Prototype  : bool ctype_upper(mixed $c)
 * Description: Checks for uppercase character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass octal and hexadecimal values to ctype_upper() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_upper() : usage variations ***\n";
$orig = setlocale(LC_CTYPE, "C");

$octal_values = vec[0101, 0102, 0103, 0104];
$hex_values =   vec[0x41, 0x42, 0x43, 0x44];

echo "\n-- Octal Values --\n";
$iterator = 1;
foreach($octal_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_upper($c));
    $iterator++;
}

echo "\n-- Hexadecimal Values --\n";
$iterator = 1;
foreach($hex_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_upper($c));
    $iterator++;
}

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
