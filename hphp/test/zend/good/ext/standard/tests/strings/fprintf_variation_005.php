<?hh
<<__EntryPoint>> function main(): void {
$int_numbers = varray[ 0, 1, -1, 2.7, -2.7, 23333333, -23333333, "1234" ];

/* creating dumping file */
$data_file = __SystemLib\hphp_test_tmppath('fprintf_variation_005.phpt.txt');
if (!($fp = fopen($data_file, 'wt')))
   return;

/* %e type variations */
fprintf($fp, "\n*** Testing fprintf() for scientific type ***\n");
foreach( $int_numbers as $num ) {
  fprintf( $fp, "\n");
  fprintf( $fp, "%e", $num );
}

fclose($fp);

print_r(file_get_contents($data_file));
echo "\nDone";

unlink($data_file);
}
