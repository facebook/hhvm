<?hh
/* Prototype: float floatval( mixed $var );
 * Description: Returns the float value of var.
 */



// get a resource type variable
<<__EntryPoint>> function main(): void {
$fp = fopen (__FILE__, "r");
fclose($fp);
$dfp = opendir ( dirname(__FILE__) );
closedir($dfp);

// other types in an array
$not_float_types = dict[
           "-2147483648" => -2147483648, // max negative integer value
           "2147483647" => 2147483648,  // max positive integer value
           "file resoruce" => $fp,
           "directory resource" => $dfp,
           "\"0.0\"" => "0.0", // string
           "\"1.0\"" => "1.0",
           "\"-1.3e3\"" => "-1.3e3",
           "\"bob-1.3e3\"" => "bob-1.3e3",
           "\"10 Some dollars\"" => "10 Some dollars",
           "\"10.2 Some Dollars\"" => "10.2 Some Dollars",
           "\"10.0 dollar\" + 1" => HH\Lib\Legacy_FIXME\cast_for_arithmetic("10.0 dollar") + 1,
           "\"10.0 dollar\" + 1.0" => HH\Lib\Legacy_FIXME\cast_for_arithmetic("10.0 dollar") + 1.0,
           "\"\"" => "",
           "true" => true,
           "NULL" => NULL,
           "null" => null,
                 ];
/* loop through the $not_float_types to see working of
   floatval() on non float types, expected output: float value valid floating point numbers */
echo "\n*** Testing floatval() on non floating types ***\n";
foreach ($not_float_types as $key => $type ) {
   echo "\n-- Iteration : $key --\n";
   var_dump( floatval($type) );
}

echo "\n*** Testing doubleval() on non floating types ***\n";

/* loop through the $not_float_types to see working of
   doubleval() on non float types, expected output: float value valid floating point numbers */
foreach ($not_float_types as $key => $type ) {
   echo "\n-- Iteration : $key --\n";
   var_dump( doubleval($type) );
}
echo "===DONE===\n";
}
