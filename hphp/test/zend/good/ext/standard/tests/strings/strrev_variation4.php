<?hh
/* Prototype  : string strrev(string $str);
 * Description: Reverse a string
 * Source code: ext/standard/string.c
*/

/* Testing strrev() function with unexpected inputs for 'str' */

//class declaration
class sample {
  public function __toString():mixed{
    return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strrev() : unexpected inputs for 'str' ***\n";
//get the resource
$resource = fopen(__FILE__, "r");


//array of values to iterate over
$values = vec[
  // empty data
  "",
  '',
];

// loop through each element of the array for str

$count = 1;
foreach($values as $value) {
  echo "\n-- Iterator $count --\n";
  try { var_dump( strrev($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count++;
};

fclose($resource);  //closing the file handle

echo "*** Done ***";
}
