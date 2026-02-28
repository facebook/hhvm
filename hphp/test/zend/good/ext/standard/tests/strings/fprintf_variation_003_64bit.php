<?hh
<<__EntryPoint>> function main(): void {
$int_numbers = vec[ 0, 1, -1, 2.7, -2.7, 23333333, -23333333, "1234" ];

/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'fprintf_variation_003_64bit.phpt.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

/* binary type variations */
fprintf($fp, "\n*** Testing fprintf() with binary ***\n");
foreach( $int_numbers as $bin_num ) {
  fprintf( $fp, "\n");
  fprintf( $fp, "%b", $bin_num );
}

fclose($fp);

print_r(file_get_contents($data_file));
echo "\nDone";

unlink($data_file);
}
