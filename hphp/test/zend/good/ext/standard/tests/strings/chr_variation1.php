<?hh

/* Prototype  : string chr  ( int $ascii  )
 * Description: Return a specific character
 * Source code: ext/standard/string.c
*/

//defining a class
class sample  {
  public function __toString() :mixed{
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing chr() function: with unexpected inputs for 'ascii' argument ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$inputs =  vec[

          // integer values
/*1*/      0,
          1,
          255,
          256,

          // float values
/*5*/      10.5,
          -20.5,
          1.1234e6,

          // array values
/*8*/      vec[],
          vec[0],
          vec[1, 2],

          // boolean values
/*11*/      true,
          false,
          TRUE,
          FALSE,

          // null values
/*15*/      NULL,
          null,

          // objects
/*17*/      new sample(),

          // resource
/*18*/      $file_handle,
];

// loop through with each element of the $inputs array to test chr() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  var_dump( bin2hex(chr($input)) );
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
