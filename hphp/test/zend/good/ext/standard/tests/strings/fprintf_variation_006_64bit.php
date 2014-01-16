<?php

$int_numbers = array( 0, 1, -1, 2.7, -2.7, 23333333, -23333333, "1234" );

/* creating dumping file */
$data_file = dirname(__FILE__) . '/fprintf_variation_006_64bit.phpt.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

/* unsigned int type variation */
fprintf($fp, "\n*** Testing fprintf() for unsigned integers ***\n");
foreach( $int_numbers as $unsig_num ) {
  fprintf( $fp, "\n");
  fprintf( $fp, "%u", $unsig_num );
}

fclose($fp);

print_r(file_get_contents($data_file));
echo "\nDone";

unlink($data_file);

?>