<?hh
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'gzeof_variation1.txt.gz';
$h = gzopen($filename, 'w');
$str = "Here is the string to be written. ";
$length = 10;
gzwrite( $h, $str );
var_dump(gzeof($h));
gzwrite( $h, $str, $length);
var_dump(gzeof($h));
gzclose($h);
var_dump(gzeof($h));
unlink($filename);
echo "===DONE===\n";
}
