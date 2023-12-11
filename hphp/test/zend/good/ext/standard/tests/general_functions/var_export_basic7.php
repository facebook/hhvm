<?hh
/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable
 * Source code: ext/standard/var.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing var_export() with valid null values ***\n";

// different valid  null vlaues
$null_var = NULL;

$valid_nulls = dict[
                "NULL" =>  NULL,
                "null" => null,
                "null_var" => $null_var,
];

/* Loop to check for above null values with var_export() */
echo "\n*** Output for null values ***\n";
foreach($valid_nulls as $key => $null_value) {
    echo "\n-- Iteration: $key --\n";
    var_export( $null_value );
    echo "\n";
    var_export( $null_value, FALSE);
    echo "\n";
    var_dump( var_export( $null_value, true) );
    echo "\n";
}
echo "===DONE===\n";
}
