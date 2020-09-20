<?hh
/* Prototype  : mixed date_sunset(mixed time [, int format [, float latitude [, float longitude [, float zenith [, float gmt_offset]]]]])
 * Description: Returns time of sunset for a given day and location
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */

// define some classes
class classWithToString
{
    public function __toString() {
        return "Class A object";
    }
}

class classWithoutToString
{
}
<<__EntryPoint>> function main(): void {
echo "*** Testing date_sunset() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
date_default_timezone_set("Asia/Calcutta");
$time = mktime(8, 8, 8, 8, 8, 2008);
$latitude = 22.34;
$zenith = 90.0;
$gmt_offset = 5.5;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

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

      // string data
      'string DQ' => "string",
      'string SQ' => 'string',
      'mixed case string' => "sTrInG",
      'heredoc' => $heredoc,

      // object data
      'instance of classWithToString' => new classWithToString(),
      'instance of classWithoutToString' => new classWithoutToString(),

      // undefined data
      'undefined var' => @$undefined_var,

      // unset data
      'unset var' => @$unset_var,
];

// loop through each element of the array for longitude

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      try { var_dump( date_sunset($time, SUNFUNCS_RET_STRING, $latitude, $value, $zenith, $gmt_offset) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      try { var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE, $latitude, $value, $zenith, $gmt_offset) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      try { var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP, $latitude, $value, $zenith, $gmt_offset) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "===DONE===\n";
}
