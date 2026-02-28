<?hh
/* Prototype  : proto string base64_decode(string str[, bool strict])
 * Description: Decodes string using MIME base64 algorithm
 * Source code: ext/standard/base64.c
 * Alias to functions:
 */

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
    echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
<<__EntryPoint>> function main(): void {
set_error_handler(test_error_handler<>);
echo "*** Testing base64_decode() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$strict = true;

//getting the resource
$file_handle = fopen(__FILE__, "r");


//array of values to iterate over
$values =  dict[
    // empty data
    "\"\"" => "",
    "''" => '',
];

// loop through each element of the array for str argument

foreach($values as $key=>$value) {
    echo "\n-- Arg value $key --\n";
        $output = null;
    try { $output =  base64_decode($value, $strict); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

    if (is_string($output)) {
        var_dump(bin2hex($output));
    } else {
        var_dump($output);
    }
};
echo "===Done===";
}
