<?hh
/* Prototype  : string strtr(string $str, string $from[, string $to]);
                string strtr(string $str, array $replace_pairs);
 * Description: Translates characters in str using given translation tables
 * Source code: ext/standard/string.c
*/

/* Test strtr() function: with unexpected inputs for 'replace_pairs'
 *  and expected type for 'str' arguments
*/

//defining a class
class sample  {
  public function __toString() :mixed{
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strtr() function: with unexpected inputs for 'replace_pairs' ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

//defining 'str' argument
$str = "012atm";

// array of inputs for 'replace_pairs' argument
$replace_pairs_arr =  vec[

  // integer values
  0,
  1,
  -2,

  // float values
  10.5,
  -20.5,
  10.5e10,

  // array values
  vec[],
  vec[0],
  vec[1, 2],

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // null vlaues
  NULL,
  null,

  // objects
  new sample(),

  // resource
  $file_handle,


];

// loop through with each element of the $replace_pairs array to test strtr() function
$count = 1;
for($index = 0; $index < count($replace_pairs_arr); $index++) {
  echo "\n-- Iteration $count --\n";
  $replace_pairs = $replace_pairs_arr[$index];
  var_dump( strtr($str, $replace_pairs) );
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***";
}
