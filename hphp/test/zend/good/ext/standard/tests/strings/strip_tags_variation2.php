<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing functionality of strip_tags() by giving unexpected values for $allowable_tags argument
*/

//get a class
class classA{
   public function __toString():mixed{
     return "Class A Object";
   }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : usage variations ***\n";

// Initialise function argument
$string = "<html><a>hello</a></html><p>world</p><!-- COMMENT --><?hh echo hello ?>";


//get a resource variable
$fp = fopen(__FILE__, "r");

//array of values to iterate over
$values = vec[

      // int data
      0,
      1,
      12345,
      -2345,

      // float data
      10.5,
      -10.5,
      10.5e10,
      10.6E-10,
      .5,

      // array data
      'Array',
      'Array',
      'Array',
      'Array',
      'Array',

      // null data
      NULL,
      null,

      // boolean data
      true,
      false,
      TRUE,
      FALSE,

      // empty data
      "",
      '',

      // object data
      new classA(),



      // resource variable
      $fp
];

// loop through each element of the array for allowable_tags
$iterator = 1;
foreach($values as $value) {
      echo "-- Iteration $iterator --\n";
      var_dump( strip_tags($string, $value) );
      $iterator++;
};

echo "Done";
}
