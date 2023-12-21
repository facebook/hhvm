<?hh
/* Prototype  : bool checkdate  ( int $month  , int $day  , int $year  )
 * Description: Checks the validity of the date formed by the arguments.
 *              A date is considered valid if each parameter is properly defined.
 * Source code: ext/date/php_date.c
 */

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
<<__EntryPoint>> function main(): void {
echo "*** Testing checkdate() : usage variation -  unexpected values to first argument \$month***\n";


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// add arrays
$index_array = vec[1, 2, 3];
$assoc_array = dict['one' => 1, 'two' => 2];

// resource
$file_handle = fopen(__FILE__, 'r');

//array of values to iterate over
$inputs = dict[
      // float data
      'float 10.5' => 10.5,
      'float -10.5' => -10.5,
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



      // resource
      'resource' => $file_handle
];

$day = 2;
$year = 1963;

foreach($inputs as $variation =>$month) {
      echo "\n-- $variation --\n";
      try { var_dump( checkdate($month, $day, $year) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

// closing the resource
fclose( $file_handle);

echo "===DONE===\n";
}
