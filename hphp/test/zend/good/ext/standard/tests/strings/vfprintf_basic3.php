<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : basic functionality - using float format ***\n";

// Initialise all required variables

$format = "format";
$format1 = "%f";
$format2 = "%f %f";
$format3 = "%f %f %f";

$format11 = "%F";
$format22 = "%F %F";
$format33 = "%F %F %F";
$arg1 = vec[11.11];
$arg2 = vec[11.11,22.22];
$arg3 = vec[11.11,22.22,33.33];

/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'vfprintf_basic3.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

vfprintf($fp, $format1,$arg1);
fprintf($fp, "\n"); 

vfprintf($fp,$format11,$arg1);
fprintf($fp, "\n"); 

vfprintf($fp,$format2,$arg2);
fprintf($fp, "\n"); 

vfprintf($fp,$format22,$arg2);
fprintf($fp, "\n"); 

vfprintf($fp,$format3,$arg3);
fprintf($fp, "\n"); 

vfprintf($fp, $format33,$arg3);
fprintf($fp, "\n"); 

fclose($fp);
print_r(file_get_contents($data_file));

unlink($data_file);
echo "===DONE===\n";
}
