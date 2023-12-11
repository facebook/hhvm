<?hh
/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable
 * Source code: ext/standard/var.c
 * Alias to functions:
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing var_export() with valid strings ***\n";
// different valid  string
$valid_strings = dict[
            "\"\"" => "",
            "\" \"" => " ",
            "''" => '',
            "' '" => ' ',
            "\"string\"" => "string",
            "'string'" => 'string',
            "\"\\0Hello\\0 World\\0\"" => "\0Hello\0 World\0",
            "\"NULL\"" => "NULL",
            "'null'" => 'null',
            "\"FALSE\"" => "FALSE",
            "'false'" => 'false',
            "\"\\x0b\"" => "\x0b",
            "\"\\0\"" => "\0",
            "'\\0'" => '\0',
            "'\\060'" => '\060',
            "\"\\070\"" => "\070"
];

/* Loop to check for above strings with var_export() */
echo "\n*** Output for strings ***\n";
foreach($valid_strings as $key => $str) {
    echo "\n-- Iteration: $key --\n";
    var_export( $str );
    echo "\n";
    var_export( $str, FALSE);
    echo "\n";
    var_dump( var_export( $str, TRUE) );
    echo "\n";
}

echo "===DONE===\n";
}
