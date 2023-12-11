<?hh
/* Prototype  : array array_flip(array $input)
 * Description: Return array with key <-> value flipped
 * Source code: ext/standard/array.c
*/

//class definition for object variable
class MyClass
{
  public function __toString()
:mixed  {
     return 'object';
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_flip() : usage variations - unexpected values for 'input' ***\n";


//resource variable
$fp = fopen(__FILE__,'r');

//array of values for 'input' argument
$values = vec[
          // int data
  /*1*/   0,
          1,
          12345,
          -2345,

          // float data
  /*5*/   10.5,
          -10.5,
          10.5e10,
          10.6E-10,
          .5,

          // null data
  /*10*/  NULL,
          null,

          // boolean data
  /*12*/  true,
          false,
          TRUE,
  /*15*/  FALSE,

          // empty data
          "",
          '',

          // string data
          "string",
          'string',

          // object data
  /*20*/  new MyClass(),



          //resource data
  /*21*/  $fp
];

// loop through each element of $values for 'input' argument
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count + 1). " --\n";
  var_dump( array_flip($values[$count]) );
};

//closing resource
fclose($fp);

echo "Done";
}
