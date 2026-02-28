<?hh
/* Prototype  : array gettimeofday([bool get_as_float])
 * Description: Returns the current time as array
 * Source code: ext/standard/microtime.c
 * Alias to functions:
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
echo "*** Testing gettimeofday() : usage variation ***\n";

date_default_timezone_set("Asia/Calcutta");

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

// loop through each element of the array for get_as_float

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      try { var_dump( gettimeofday($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "===DONE===\n";
}
