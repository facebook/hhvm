<?hh
/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable
 * Source code: ext/standard/var.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing var_export() with integer values ***\n";
// different integer vlaues
$valid_ints = dict[
                '0' => '0',
                '1' => '1',
                '-1' => '-1',
                '-2147483648' => '-2147483648', // max negative integer value
                '-2147483647' => '-2147483647',
                '2147483647' => 2147483647,  // max positive integer value
                '2147483640' => 2147483640,
                '0x123B' => 0x123B,      // integer as hexadecimal
                "'0x12ab'" => '0x12ab',
                "'0Xfff'" => '0Xfff',
                "'0XFA'" => '0XFA',
                "-0x80000000" => -0x80000000, // max negative integer as hexadecimal
                "'0x7fffffff'" => '0x7fffffff',  // max postive integer as hexadecimal
                "0x7FFFFFFF" => 0x7FFFFFFF,  // max postive integer as hexadecimal
                "'0123'" => '0123',        // integer as octal
                "-020000000000" => -020000000000, // max negative integer as octal
                "017777777777" => 017777777777,  // max positive integer as octal
];

/* Loop to check for above integer values with var_export() */
echo "\n*** Output for integer values ***\n";
foreach($valid_ints as $key => $int_value) {
    echo "\n-- Iteration: $key --\n";
    var_export( $int_value );
    echo "\n";
    var_export( $int_value, FALSE);
    echo "\n";
    var_dump( var_export( $int_value, TRUE) );
}

echo "===DONE===\n";
}
