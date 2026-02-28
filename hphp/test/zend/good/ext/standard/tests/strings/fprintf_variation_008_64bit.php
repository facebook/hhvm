<?hh
<<__EntryPoint>> function main(): void {
$int_variation = vec[ "%d", "%-d", "%+d", "%7.2d", "%-7.2d", "%07.2d", "%-07.2d", "%'#7.2d" ];
$int_numbers = vec[ 0, 1, -1, 2.7, -2.7, 23333333, -23333333, "1234" ];

/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'fprintf_variation_008_64bit.phpt.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

/* hexadecimal type variations */
fprintf($fp, "\n*** Testing fprintf() for hexadecimals ***\n");
foreach( $int_numbers as $hexa_num ) {
 fprintf( $fp, "\n");
 fprintf( $fp, "%x", $hexa_num );
}

fclose($fp);

print_r(file_get_contents($data_file));
echo "\nDone";

unlink($data_file);
}
