<?php

$float_variation = array( "%f","%-f", "%+f", "%7.2f", "%-7.2f", "%07.2f", "%-07.2f", "%'#7.2f" );
$float_numbers = array( 0, 1, -1, 0.32, -0.32, 3.4. -3.4, 2.54, -2.54 );

/* creating dumping file */
$data_file = dirname(__FILE__) . '/fprintf_variation_001.phpt.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

$counter = 1;
/* float type variations */
fprintf($fp, "\n*** Testing fprintf() with floats ***\n");

foreach( $float_variation as $float_var ) {
  fprintf( $fp, "\n-- Iteration %d --\n",$counter);
  foreach( $float_numbers as $float_num ) {
    fprintf( $fp, "\n");
    fprintf( $fp, $float_var, $float_num );
  }
  $counter++;
}

fclose($fp);
print_r(file_get_contents($data_file));
echo "\nDone";

unlink($data_file);

?>