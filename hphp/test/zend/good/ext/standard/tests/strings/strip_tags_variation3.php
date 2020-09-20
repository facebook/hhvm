<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing functionality of strip_tags() by giving unexpected values for $str and $allowable_tags arguments
*/

// get a class
class classA{
  public function __toString(){
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : usage variations ***\n";

// get a resource
$fp=fopen(__FILE__, "r");

// get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = varray[

      // int data
      0,
      1,
      12345,
      -2345,

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // array data
      varray[],
      varray[0],
      varray[1],
      varray[1, 2],
      darray['color' => 'red', 'item' => 'pen'],

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

      // undefined data
      @$undefined_var,

      // unset data
      @$unset_var,

      // resource variable
      $fp

];

// loop through each element of the array for allowable_tags
$iterator = 1;
foreach($values as $value) {
      echo "-- Iteration $iterator --\n";
      try { var_dump( strip_tags($value, $value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      $iterator++;
};

echo "===DONE===\n";
}
