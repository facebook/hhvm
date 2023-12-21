<?hh
/* Prototype  : string str_pad  ( string $input  , int $pad_length  [, string $pad_string  [, int $pad_type  ]] )
 * Description: Pad a string to a certain length with another string
 * Source code: ext/standard/string.c
*/

/* Test str_pad() function: with unexpected inputs for '$pad_length'
 *  and expected type for '$input'
*/

//defining a class
class sample  {
  public function __toString() :mixed{
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing str_pad() function: with unexpected inputs for 'pad_length' argument ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$pad_lengths =  vec[

          // integer values
/*1*/      0,
          1,
          -2,
          255,

          // float values
/*5*/      10.5,
          -20.5,
          10.12345e2,

          // array values
/*8*/      vec[],
          vec[0],
          vec[1, 2],

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
];

//defining '$input' argument
$input = "Test string";

// loop through with each element of the $pad_lengths array to test str_pad() function
$count = 1;
foreach($pad_lengths as $pad_length) {
  echo "-- Iteration $count --\n";
  try { var_dump( str_pad($input, $pad_length) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
