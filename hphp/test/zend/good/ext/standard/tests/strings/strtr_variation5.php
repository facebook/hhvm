<?hh
/* Prototype  : string strtr(string $str, string $from[, string $to]);
                string strtr(string $str, array $replace_pairs);
 * Description: Translates characters in str using given translation tables
 * Source code: ext/standard/string.c
*/

/* Test strtr() function: with unexpected inputs for 'str'
 *  and expected type for 'from' & 'to' arguments
*/

//defining a class
class sample  {
  public function __toString() :mixed{
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strtr() function: with unexpected inputs for 'str' ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values
$strings =  varray [

          // integer values
/*1*/      0,
          1,
          -2,

          // float values
/*4*/      10.5,
          -20.5,
          10.1234567e10,

          // array values
/*7*/      vec[],
          vec[0],
          vec[1, 2],

          // boolean values
/*10*/      true,
          false,
          TRUE,
          FALSE,

          // null vlaues
/*14*/      NULL,
          null,

          // objects
/*16*/      new sample(),

          // resource
/*17*/      $file_handle,


];

//defining 'from' argument
$from = "012atm";

//defining 'to' argument
$to = "atm012";

// loop through with each element of the $strings array to test strtr() function
$count = 1;
for($index = 0; $index < count($strings); $index++) {
  echo "-- Iteration $count --\n";
  $str = $strings[$index];
  try { var_dump( strtr($str, $from, $to) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
