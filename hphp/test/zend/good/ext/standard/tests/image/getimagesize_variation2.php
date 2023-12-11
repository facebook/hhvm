<?hh
/* Prototype  : proto array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array
 * Source code: ext/standard/image.c
 * Alias to functions:
 */

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
    echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
<<__EntryPoint>> function main(): void {
set_error_handler(test_error_handler<>);
echo "*** Testing getimagesize() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$imagefile = dirname(__FILE__)."/test1pix.jpg";

//array of values to iterate over
$values = dict[

      // int data
      "0" => 0,
      "1" => 1,
      "12345" => 12345,
      "-2345" => -2345,

      // float data
      "10.5" => 10.5,
      "-10.5" => -10.5,
      "10.1234567e5" => 10.1234567e10,
      "10.7654321e-5" => 10.7654321E-5,
      0 => .5,

      // array data
      "array()" => vec[],
      "array(0)" => vec[0],
      "array(1)" => vec[1],
      "array(1, 2)" => vec[1, 2],
      "array('color' => 'red', 'item' => 'pen')" => dict['color' => 'red', 'item' => 'pen'],

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
      "new stdClass()" => new stdClass()
];

// loop through each element of the array for info

foreach($values as $key => $value) {
      echo "\n-- Arg value $key --\n";
      getimagesize($imagefile, inout $value);
      var_dump(bin2hex($value["APP0"]));
};

echo "===DONE===\n";
}
