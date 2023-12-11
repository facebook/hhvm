<?hh

/* Prototype  : int strlen  ( string $string  )
 * Description: Get string length
 * Source code: ext/standard/string.c
*/

//defining a class
class sample  {
  public function __toString() :mixed{
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strlen() : with unexpected input for 'string' argument ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$inputs =  varray [

          // integer values
/*1*/      0,
          1,
          -2,
          2147483647,
          -2147483648,

          // float values
/*6*/      10.5,
          -20.5,
          10.1234567e10,

          // array values
/*9*/      vec[],
          vec[0],
          vec[1, 2],

          // boolean values
/*12*/      true,
          false,
          TRUE,
          FALSE,

          // null values
/*16*/      NULL,
          null,

          // objects
/*18*/      new sample(),

          // resource
/*19*/      $file_handle,
];

//defining '$pad_length' argument
$pad_length = "20";

// loop through with each element of the $inputs array to test strlen() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  try { var_dump( strlen($input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
