<?hh
/* Prototype  : string money_format  ( string $format  , float $number  )
 * Description: Formats a number as a currency string
 * Source code: ext/standard/string.c
*/

// ===========================================================================================
// = We do not test for exact return-values, as those might be different between OS-versions =
// ===========================================================================================

//defining a couple of sample classes
class class_no_tostring  {
}

class class_with_tostring  {
  public function __toString() :mixed{
    return "  sample object  ";
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing money_format() function: with unexpected inputs for 'number' argument ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $number
$numbers =  vec[

          // integer values
/*1*/      0,
          1,
          255,
          256,
          2147483647,
          -2147483648,

          // float values
/*7*/      10.5,
          -20.5,
          10.1234567e10,

          // array values
/*10*/      vec[],
          vec[0],
          vec[1, 2],

          // boolean values
/*13*/      true,
          false,
          TRUE,
          FALSE,

          // null values
/*17*/      NULL,
          null,

          // string values
/*19*/      "abcd",
          'abcd',
          "0x12f",
          "%=*!14#8.2nabcd",

          // objects
/*23*/      new class_no_tostring(),
          new class_with_tostring(),

          // resource
/*25*/      $file_handle,
];

// loop through with each element of the $numbers array to test money_format() function
$count = 1;
$format = '%14#8.2i';

foreach($numbers as $number) {
  echo "-- Iteration $count --\n";
  try { echo gettype(money_format($format, $number))."\n"; } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

// close the file handle
fclose($file_handle);
echo "===Done===";
}
