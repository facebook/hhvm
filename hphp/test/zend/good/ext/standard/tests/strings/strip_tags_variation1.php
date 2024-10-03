<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing functionality of strip_tags() by giving unexpected input values for $str argument
*/

//get a class
class classA{
  public function __toString():mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : usage variations ***\n";

//get a resource variable
$fp = fopen(__FILE__, "r");

//array of values to iterate over
$values = vec[
          // empty data
/*21*/    "",
          '',
];

// loop through each element of the array for allowable_tags
$iterator = 1;
foreach($values as $value) {
      echo "-- Iteration $iterator --\n";
      try { var_dump( strip_tags($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      $iterator++;
};

echo "===DONE===\n";
}
