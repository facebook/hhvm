<?hh
/* Prototype: string implode ( string $glue, array $pieces );
   Description: Returns a string containing a string representation of all the
                array elements in the same order, with the glue string between each element.
*/

class foo
{
  function __toString() :mixed{
    return "Object";
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing implode() for basic opeartions ***\n";
$arrays = varray [
  vec[1,2],
  vec[1.1,2.2],
  vec[false,true],
  vec[],
  vec["a","aaaa","b","bbbb","c","ccccccccccccccccccccc"]
];
/* loop to output string with ', ' as $glue, using implode() */
foreach ($arrays as $array) {
  var_dump( implode(', ', $array) );
  var_dump($array);
}

echo "\n*** Testing implode() with variations of glue ***\n";
/* checking possible variations */
$pieces = varray [
  2,
  0,
  -639,
  true,
  "PHP",
  false,
  NULL,
  "",
  " ",
  "string\x00with\x00...\0"
];
$glues = varray [
  "TRUE",
  true,
  false,
  "",
  " ",
  "string\x00between",
  NULL,
  -0,
  '\0'
];
/* loop through to display a string containing all the array $pieces in the same order,
   with the $glue string between each element  */
$counter = 1;
foreach($glues as $glue) {
  echo "-- Iteration $counter --\n";
  var_dump( implode($glue, $pieces) );
  $counter++;
}

/* empty string */
echo "\n*** Testing implode() on empty string ***\n";
var_dump( implode("") );

echo "\n*** Testing implode() on objects ***\n";
/* checking on objects */
$obj = new foo(); //creating new object
$arr = vec[$obj, $obj];
var_dump( implode(",", $arr) );
var_dump($arr);

/* Checking on resource types */
echo "\n*** Testing end() on resource type ***\n";
/* file type resource */
$file_handle = fopen(__FILE__, "r");

/* directory type resource */
$dir_handle = opendir( dirname(__FILE__) );

/* store resources in array for comparison */
$resources = vec[$file_handle, $dir_handle];

var_dump( implode("::", $resources) );

echo "\n*** Testing error conditions ***\n";
/* zero argument */
try { var_dump( implode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* only glue */
var_dump( implode("glue") );

/* int as pieces */
var_dump( implode("glue",1234) );

/* NULL as pieces */
var_dump( implode("glue", NULL) );

/* pieces as NULL array */
var_dump( implode(",", vec[NULL]) );

/* integer as glue */
var_dump( implode(12, "pieces") );

/* NULL as glue */
var_dump( implode(NULL, "abcd") );

/* args > than expected */
try { var_dump( implode("glue", "pieces", "extra") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* closing resource handles */
fclose($file_handle);
closedir($dir_handle);

echo "Done\n";
}
