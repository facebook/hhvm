<?hh

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	if (error_reporting() != 0) {
		// report non-silenced errors
		echo "Error: $err_no - $err_msg, $filename($linenum)\n";
	}
}

// define some classes
class classWithToString
{
	public function __toString() {
		return b"Class A object";
	}
}

class classWithoutToString
{
}
<<__EntryPoint>>
function entrypoint_mcrypt_decrypt_variation3(): void {
  /* Prototype  : string mcrypt_decrypt(string cipher, string key, string data, string mode, string iv)
   * Description: OFB crypt/decrypt data using key key with cipher cipher starting with iv 
   * Source code: ext/mcrypt/mcrypt.c
   * Alias to functions: 
   */

  echo "*** Testing mcrypt_decrypt() : usage variation ***\n";
  set_error_handler(fun('test_error_handler'));

  // Initialise function arguments not being substituted (if any)
  $cipher = MCRYPT_TRIPLEDES;
  $key = b"string_val\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
  $mode = MCRYPT_MODE_ECB;
  $iv = b'01234567';

  //get an unset variable
  $unset_var = 10;
  unset ($unset_var);

  // heredoc string
  $heredoc = b<<<EOT
hello world
EOT;

  // get a resource variable
  $fp = fopen(__FILE__, "r");

  // add arrays
  $index_array = varray [1, 2, 3];
  $assoc_array = darray ['one' => 1, 'two' => 2];

  //array of values to iterate over
  $inputs = darray[

        // int data
        'int 0' => 0,
        'int 1' => 1,
        'int 12345' => 12345,
        'int -12345' => -2345,

        // float data
        'float 10.5' => 10.5,
        'float -10.5' => -10.5,
        'float 12.3456789000e10' => 12.3456789000e10,
        'float -12.3456789000e10' => -12.3456789000e10,
        'float .5' => .5,

        // array data
        'empty array' => varray[],
        'int indexed array' => $index_array,
        'associative array' => $assoc_array,
        'nested arrays' => varray['foo', $index_array, $assoc_array],

        // null data
        'uppercase NULL' => NULL,
        'lowercase null' => null,

        // boolean data
        'lowercase true' => true,
        'lowercase false' =>false,
        'uppercase TRUE' =>TRUE,
        'uppercase FALSE' =>FALSE,

        // empty data
        'empty string DQ' => "",
        'empty string SQ' => '',

        // object data
        'instance of classWithToString' => new classWithToString(),
        'instance of classWithoutToString' => new classWithoutToString(),

        // undefined data
        'undefined var' => @$undefined_var,

        // unset data
        'unset var' => @$unset_var,

        // resource variable
        'resource' => $fp      
  ];

  // loop through each element of the array for data

  foreach($inputs as $valueType =>$value) {
        echo "\n--$valueType--\n";
        try { var_dump(bin2hex(mcrypt_decrypt($cipher, $key, $value, $mode, $iv))); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }

  fclose($fp);

  echo "===DONE===\n";
}
