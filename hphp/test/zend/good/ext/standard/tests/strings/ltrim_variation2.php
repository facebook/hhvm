<?hh

/* Prototype  : string ltrim  ( string $str  [, string $charlist  ] )
 * Description: Strip whitespace (or other characters) from the beginning of a string.
 * Source code: ext/standard/string.c
*/

//defining a class
class sample  {
  public function __toString() {
    return "  sample object  ";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing ltrim() function: with unexpected inputs for 'charlist' argument ***\n";

//get an unset variable
$unset_var = '  string_val  ';
unset($unset_var);

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$inputs =  varray [

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
/*10*/      varray[],
          varray[0],
          varray[1, 2],

          // boolean values
/*13*/      true,
          false,
          TRUE,
          FALSE,

          // null values
/*17*/      NULL,
          null,

          // objects
/*19*/      new sample(),

          // resource
/*20*/      $file_handle,

          // undefined variable
/*21*/      @$undefined_var,

          // unset variable
/*22*/      @$unset_var
];

// loop through with each element of the $inputs array to test ltrim() function
$count = 1;
foreach($inputs as $charlist) {
  echo "-- Iteration $count --\n";
  // strip white space and any "minus" signs
  try { var_dump( ltrim("!---Hello World---!", $charlist) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
