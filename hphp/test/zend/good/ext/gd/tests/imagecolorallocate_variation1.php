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
