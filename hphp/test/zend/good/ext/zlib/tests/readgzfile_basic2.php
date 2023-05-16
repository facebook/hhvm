<?hh <<__EntryPoint>> function main(): void {
$plaintxt = b<<<EOT
hello world
is a very common test
for all languages

EOT;
$dirname = sys_get_temp_dir().'/'.'readgzfile_temp';
$filename = $dirname.'/readgzfile_basic2.txt';
mkdir($dirname);
$h = fopen($filename, 'w');
fwrite($h, $plaintxt);
fclose($h);


var_dump(readgzfile( $filename ) );

unlink($filename);
rmdir($dirname);
echo "===DONE===\n";
}
