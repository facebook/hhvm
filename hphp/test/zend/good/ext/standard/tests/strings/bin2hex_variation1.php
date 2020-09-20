<?hh

/* Prototype  : string bin2hex  ( string $str  )
 * Description: Convert binary data into hexadecimal representation
 * Source code: ext/standard/string.c
*/

//defining a class
class sample  {
  public function __toString() {
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing bin2hex() function: with unexpected inputs for 'str' argument ***\n";

//get an unset variable
$unset_var = 'string_val';
unset($unset_var);

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$inputs =  varray [

      // integer values
/*1*/ 0,
      1,
      123456,

      // float values
/*4*/ 10.5,
      -20.5,
      10.1234567e10,

      // array values
/*7*/ varray[],
      varray[0],
      varray[1, 2],

      // boolean values
/*10*/true,
      false,
      TRUE,
      FALSE,

      // null values
/*14*/NULL,
      null,

      // objects
/*16*/new sample(),

      // resource
/*17*/$file_handle,

      // undefined variable
/*18*/@$undefined_var,

      // unset variable
/*19*/@$unset_var
];

// loop through with each element of the $inputs array to test bin2hex() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  try { var_dump(bin2hex($input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
