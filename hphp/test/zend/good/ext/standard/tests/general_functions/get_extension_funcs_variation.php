<?hh
/* Prototype  : array get_extension_funcs  ( string $module_name  )
 * Description: Returns an array with the names of the functions of a module.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

//defining a class
class sample  {
  public function __toString() :mixed{
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing get_extension_funcs() function: with unexpected inputs for 'module_name' argument ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $str
$inputs =  varray [

  // integer values
  0,
  1,
  255,
  256,
  PHP_INT_MAX,
  -PHP_INT_MAX,

  // float values
  10.5,
  -20.5,
  10.1234567e10,

  // array values
  vec[],
  vec[0],
  vec[1, 2],

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // null values
  NULL,
  null,

  // objects
  new sample(),

  // resource
  $file_handle,


];

// loop through with each element of the $inputs array to test get_extension_funcs() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  try { var_dump( get_extension_funcs($input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
