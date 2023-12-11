<?hh
/*
 Prototype: string fgetc ( resource $handle );
 Description: Gets character from file pointer
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fgetc() with zero argument --\n";
try { var_dump( fgetc() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing fgetc() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
try { var_dump( fgetc($fp, $fp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose($fp);

// test invalid arguments : non-resources
echo "-- Testing fgetc() with invalid arguments --\n";
$invalid_args = varray [
  "string",
  10,
  10.5,
  true,
  vec[1,2,3],
  new stdClass,
];
/* loop to test fgetc() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  try { var_dump( fgetc($invalid_args[$loop_counter - 1]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

echo "Done\n";
}
