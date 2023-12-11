<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : basic functionality - using string format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%s\n";
$format2 = "%s %s\n";
$format3 = "%s %s %s\n";
$arg1 = vec["one"];
$arg2 = vec["one","two"];
$arg3 = vec["one","two","three"];


/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'vfprintf_basic1.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

$result = vfprintf($fp, $format1, $arg1);
var_dump($result);
$result = vfprintf($fp, $format2, $arg2);
var_dump($result);
$result = vfprintf($fp, $format3, $arg3);
var_dump($result);

fclose($fp);
print_r(file_get_contents($data_file));

unlink($data_file);

echo "===DONE===\n";
}
