<?hh
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'fstat.dat';

$fp = fopen($filename, "w");
var_dump(fstat($fp));
fclose($fp);
var_dump(fstat($fp));

unlink($filename);
echo "Done\n";
}
