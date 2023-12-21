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
function entrypoint_imagecolorallocate_variation1(): void {
  /* Prototype  : int imagecolorallocate(resource im, int red, int green, int blue)
   * Description: Allocate a color for an image
   * Source code: ext/gd/gd.c
   */

  echo "*** Testing imagecolorallocate() : usage variations ***\n";

  // Initialise function arguments not being substituted (if any)
  $red = 10;
  $green = 10;
  $blue = 10;

  $fp = tmpfile();


  // heredoc string
  $heredoc = <<<EOT
hello world
EOT;

  // add arrays
  $index_array = vec[1, 2, 3];
  $assoc_array = dict['one' => 1, 'two' => 2];

  //array of values to iterate over
  $values = dict[

        // int data
        'int 0' => 0,
        'int 1' => 1,
        'int 12345' => 12345,
        'int -12345' => -12345,

        // float data
        'float 10.5' => 10.5,
        'float -10.5' => -10.5,
        'float 10.1234567e10' => 10.1234567e10,
        'float 10.7654321E-10' => 10.7654321E-10,
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



        //resource 
        "file resource" => $fp
  ];

  // loop through each element of the array for im
  foreach($values as $key => $value) {
        echo "\n-- $key --\n";
        try { var_dump( imagecolorallocate($value, $red, $green, $blue) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }
  echo "===DONE===\n";
}
