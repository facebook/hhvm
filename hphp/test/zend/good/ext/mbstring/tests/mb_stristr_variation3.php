<?hh

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
	if (error_reporting() != 0) {
		// report non-silenced errors
		echo "Error: $err_no - $err_msg, $filename($linenum)\n";
	}
}

// define some classes
class classWithToString
{
	public function __toString() :mixed{
		return b"Class A object";
	}
}

class classWithoutToString
{
}
<<__EntryPoint>>
function entrypoint_mb_stristr_variation3(): void {
  /* Prototype  : string mb_stristr(string haystack, string needle[, bool part[, string encoding]])
   * Description: Finds first occurrence of a string within another, case insensitive
   * Source code: ext/mbstring/mbstring.c
   * Alias to functions:
   */

  echo "*** Testing mb_stristr() : usage variation ***\n";
  set_error_handler(test_error_handler<>);

  // Initialise function arguments not being substituted (if any)
  $haystack = b'string_val';
  $needle = b'_';
  $encoding = 'utf-8';


  // heredoc string
  $heredoc = b<<<EOT
hello world
EOT;

  // add arrays
  $index_array = vec[1, 2, 3];
  $assoc_array = dict['one' => 1, 'two' => 2];

  //array of values to iterate over
  $inputs = dict[
        // boolean data
        'lowercase true' => true,
        'lowercase false' =>false,
        'uppercase TRUE' =>TRUE,
        'uppercase FALSE' =>FALSE,
  ];

  // loop through each element of the array for part

  foreach($inputs as $key =>$value) {
        echo "\n--$key--\n";
        $res = false;
        try { $res = mb_stristr($haystack, $needle, $value, $encoding); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
        if ($res === false) {
           var_dump($res);
        }
        else {
           var_dump(bin2hex($res));
        }
  }

  echo "===DONE===\n";
}
