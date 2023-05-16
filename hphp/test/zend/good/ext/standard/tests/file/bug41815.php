<?hh
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'concur_rw.txt';

@unlink($filename);
$writer = fopen($filename, "wt");
$reader = fopen($filename, "r");
fread($reader, 1);
fwrite($writer, "foo");

if (strlen(fread($reader, 10)) > 0) {
	echo "OK\n";
}

fclose($writer);
fclose($reader);

@unlink($filename);

echo "Done\n";
}
