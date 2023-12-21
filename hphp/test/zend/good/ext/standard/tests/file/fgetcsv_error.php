<?hh
/*
 Prototype: array fgetcsv ( resource $handle [, int $length [, string $delimiter [, string $enclosure [, string $escape]]]] );
 Description: Gets line from file pointer and parse for CSV fields
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fgetcsv() with zero argument --\n";
try { var_dump( fgetcsv() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing fgetcsv() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
$len = 1024;
$delim = ";";
$enclosure ="\"";
$escape = '"';
try { var_dump( fgetcsv($fp, $len, $delim, $enclosure, $escape, $fp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose($fp);

// test invalid arguments : non-resources
echo "-- Testing fgetcsv() with invalid arguments --\n";
$invalid_args = vec[
  "string",
  10,
  10.5,
  true,
  vec[1,2,3],
  new stdClass,
];
/* loop to test fgetcsv() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  try { var_dump( fgetcsv($invalid_args[$loop_counter - 1]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // with default args
  try { var_dump( fgetcsv($invalid_args[$loop_counter - 1], $len, $delim, $enclosure, $escape) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // all args specified
}

echo "Done\n";
}
