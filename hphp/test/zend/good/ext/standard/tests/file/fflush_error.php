<?php
/*
 Prototype: bool fflush ( resource $handle );
 Description: Flushes the output to a file
*/

echo "*** Testing error conditions ***\n";
$file_path = dirname(__FILE__);

// zero argument
echo "-- Testing fflush(): with zero argument --\n";
var_dump( fflush() );

// more than expected no. of args
echo "-- Testing fflush(): with more than expected number of arguments --\n";

$filename = "$file_path/fflush_error.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");
   
var_dump( fflush($file_handle, $file_handle) );
fclose($file_handle);

// test invalid arguments : non-resources
echo "-- Testing fflush(): with invalid arguments --\n";
$invalid_args = array (
  "string",
  10,
  10.5,
  true,
  array(1,2,3),
  new stdclass
);

/* loop to test fflush() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  var_dump( fflush($invalid_args[$loop_counter - 1]) );
}
echo "\n*** Done ***";
?>

<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/fflush_error.tmp");
?>
