<?hh

/* Prototype: string stristr ( string $haystack, string $needle );
   Description: Case-insensitive strstr().
*/

//defining a class
class sample  {
  public function __toString() {
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing stristr() function: with unexpected inputs for 'string' argument ***\n";

//get an unset variable
$unset_var = 'string_val';
unset($unset_var);

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$inputs =  varray [

          // integer values
/*1*/      0,
          1,
          -2,
          -PHP_INT_MAX,

          // float values
/*5*/      10.5,
          -20.5,
          10.1234567e10,

          // array values
/*8*/      varray[],
          varray[0],
          varray[1, 2],

          // boolean values
/*11*/      true,
          false,
          TRUE,
          FALSE,

          // null vlaues
/*15*/      NULL,
          null,

          // objects
/*17*/      new sample(),

          // resource
/*18*/      $file_handle,

          // undefined variable
/*19*/      @$undefined_var,

          // unset variable
/*20*/      @$unset_var
];

//defining '$pad_length' argument
$pad_length = "20";

// loop through with each element of the $inputs array to test stristr() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  try { var_dump( stristr($input, " ") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
