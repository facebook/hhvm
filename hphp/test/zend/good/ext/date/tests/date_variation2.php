<?hh
/* Prototype  : string date  ( string $format  [, int $timestamp  ] )
 * Description: Format a local time/date.
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
echo "*** Testing date() : usage variation -  unexpected values to second argument \$timestamp***\n";

//Set the default time zone
date_default_timezone_set("Europe/London");


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

      // int data
      'int 0' => 0,
      'int 1' => 1,
      'int 12345' => 12345,
      'int -12345' => -12345,

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

$format = "F j, Y, g:i a";

foreach($inputs as $variation =>$timestamp) {
      echo "\n-- $variation --\n";
            if ($timestamp === null) {
                $without_timestamp = date($format);
                $with_timestamp = date($format, $timestamp);
                // These is a risk that the time change right between these calls if so
                // we do another try.
                if ($with_timestamp !== $without_timestamp) {
                    $without_timestamp = date($format);
                }
                var_dump($with_timestamp === $without_timestamp);
            } else {
        try { var_dump( date($format, $timestamp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
            }
};

// closing the resource
fclose( $file_handle );

echo "===DONE===\n";
}
