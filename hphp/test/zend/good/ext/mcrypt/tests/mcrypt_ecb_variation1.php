<?hh

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
	if ($err_no & error_reporting()) {
		// report non-silenced errors
		echo "Error: $err_no - $err_msg, $filename($linenum)\n";
	}
}

// define some classes
class classWithToString
{
	public function __toString() :mixed{
		return "Class A object";
	}
}

class classWithoutToString
{
}
<<__EntryPoint>>
function entrypoint_mcrypt_ecb_variation1(): void {
  error_reporting(E_ALL & ~E_DEPRECATED);

  /* Prototype  : string mcrypt_ecb(string cipher, string key, string data, int mode, string iv)
   * Description: ECB crypt/decrypt data using key key with cipher cipher starting with iv
   * Source code: ext/mcrypt/mcrypt.c
   * Alias to functions:
   */

  echo "*** Testing mcrypt_ecb() : usage variation ***\n";
  set_error_handler(test_error_handler<>);

  // Initialise function arguments not being substituted (if any)
  $key = b'string_val';
  $data = b'string_val';
  $mode = MCRYPT_ENCRYPT;
  $iv = b'string_val';


  // heredoc string
  $heredoc = <<<EOT
hello world
EOT;

  //array of values to iterate over
  $inputs = dict[
        // empty data
        'empty string DQ' => "",
        'empty string SQ' => '',
  ];

  // loop through each element of the array for cipher

  foreach($inputs as $valueType =>$value) {
        echo "\n--$valueType--\n";
        try { var_dump( mcrypt_ecb($value, $key, $data, $mode, $iv) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }

  echo "===DONE===\n";
}
