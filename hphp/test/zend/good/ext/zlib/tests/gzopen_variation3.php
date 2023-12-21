<?hh

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
function entrypoint_gzopen_variation3(): void {
  /* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
   * Description: Open a .gz-file and return a .gz-file pointer
   * Source code: ext/zlib/zlib.c
   * Alias to functions:
   */

  echo "*** Testing gzopen() : usage variation ***\n";

  // Initialise function arguments not being substituted (if any)
  $filename = dirname(__FILE__)."/004.txt.gz";
  $mode = 'r';

  // heredoc string
  $heredoc = <<<EOT
hello world
EOT;

  // get a resource variable
  $fp = fopen(__FILE__, "r");

  // add arrays
  $index_array = vec[1, 2, 3];
  $assoc_array = dict['one' => 1, 'two' => 2];

  //array of values to iterate over
  $inputs = dict[

        // float data
        'float 10.5' => 10.5,
        'float -10.5' => -10.5,
        'float 12.3456789000e10' => 12.3456789000e10,
        'float -12.3456789000e10' => -12.3456789000e10,
        'float .5' => .5,

        // array data
        'empty array' => vec[],
        'int indexed array' => $index_array,
        'associative array' => $assoc_array,
        'nested arrays' => vec['foo', $index_array, $assoc_array],

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

        // string data
        'string DQ' => "string",
        'string SQ' => 'string',
        'mixed case string' => "sTrInG",
        'heredoc' => $heredoc,

        // object data
        'instance of classWithToString' => new classWithToString(),
        'instance of classWithoutToString' => new classWithoutToString(),

        // resource variable
        'resource' => $fp
  ];

  // loop through each element of the array for use_include_path

  foreach($inputs as $key =>$value) {
        echo "\n--$key--\n";
  			$res = null;
        try { $res = gzopen($filename, $mode, $value); } catch (Exception $e) { echo 'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  			var_dump($res);
        if ($res === true) {
           gzclose($res);
        }
  }

  fclose($fp);

  echo "===DONE===\n";
}
