<?hh
/* Prototype  : string quoted_printable_decode  ( string $str  )
 * Description: Convert a quoted-printable string to an 8 bit string
 * Source code: ext/standard/string.c
*/

/*
* Testing quoted_printable_decode() : with different unexpected values for format argument other than the strings
*/

// declaring class
class sample
{
  public function __toString() :mixed{
    return "Object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing quoted_printable_decode() : with unexpected values for 'str' argument ***\n";

// initialing required variables
$arg1 = "second arg";
$arg2 = "third arg";


// creating a file resource
$file_handle = fopen(__FILE__, 'r');

//array of values to iterate over
$values = vec[

                // int data
/*1*/          0,
              1,
              12345,
              -2345,

              // float data
/*5*/          10.5,
              -10.5,
              10.1234567e10,
              10.7654321E-10,
              .5,

              // array data
/*10*/          vec[],
              vec[0],
              vec[1],
              vec[1, 2],
              dict['color' => 'red', 'item' => 'pen'],

              // null data
/*15*/          NULL,
              null,

              // boolean data
/*17*/          true,
              false,
              TRUE,
              FALSE,

              // empty data
/*21*/          "",
              '',

              // object data
/*23*/          new sample(),



              // resource data
/*26*/          $file_handle
];

// loop through each element of the array for 'str'

$count = 1;
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  try { var_dump(bin2hex(quoted_printable_decode($value))); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count++;
};

// close the resource
fclose($file_handle);

echo "===DONE===\n";
}
