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
$values =  darray [
       // int data
    "0" =>  0,
    "1" =>  1,
    "12345" =>  12345,
    "-2345"    =>  -2345,

    // float data
    "10.5" =>  10.5,
    "-10.5" => -10.5,
    "10.1234567e10" =>    10.1234567e10,
    "10.7654321E-10" => 10.7654321E-10,
    ".5" => .5,

    // array data
    "array()" =>   vec[],
    "array(0)" =>  vec[0],
    "array(1)" =>  vec[1],
    "array(1, 2)" => vec[1, 2],
    "array('color' => 'red', 'item' => 'pen'" => dict['color' => 'red', 'item' => 'pen'],

    // null data
    "NULL" => NULL,
    "null" => null,

    // boolean data
    "true" => true,
    "false" => false,
    "TRUE" => TRUE,
    "FALSE" => FALSE,

    // empty data
    "\"\"" => "",
    "''" => '',

    // object data
    "stdClass object" => new stdClass(),



    // resource data
    "resource" => $file_handle
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
