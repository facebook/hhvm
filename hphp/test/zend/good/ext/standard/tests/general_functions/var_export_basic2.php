<?hh

/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable
 * Source code: ext/standard/var.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing var_export() with valid boolean values ***\n";
// different valid  boolean vlaues
$valid_bool = dict[
            "1" => 1,
            "TRUE" => TRUE,
            "true" => true,
            "0" => 0,
            "FALSE" => FALSE,
            "false" => false
];

/* Loop to check for above boolean values with var_export() */
echo "\n*** Output for boolean values ***\n";
foreach($valid_bool as $key => $bool_value) {
    echo "\n-- Iteration: $key --\n";
    var_export( $bool_value );
    echo "\n";
    var_export( $bool_value, FALSE);
    echo "\n";
    var_dump( var_export( $bool_value, TRUE) );
    echo "\n";
}
echo "===DONE===\n";
}
