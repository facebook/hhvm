<?hh
<<__EntryPoint>> function main(): void {
$filename = __SystemLib\hphp_test_tmppath('fstat.dat');

$fp = fopen($filename, "w");
var_dump(fstat($fp));
fclose($fp);
var_dump(fstat($fp));

@unlink($filename);
echo "Done\n";
}
