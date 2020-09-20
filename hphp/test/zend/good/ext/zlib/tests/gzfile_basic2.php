<?hh <<__EntryPoint>> function main(): void {
$plaintxt = b<<<EOT
hello world
is a very common test
for all languages
EOT;
$dirname = __SystemLib\hphp_test_tmppath('gzfile_temp');
$filename = $dirname.'/gzfile_basic2.txt';
mkdir($dirname);
$h = fopen($filename, 'w');
fwrite($h, $plaintxt);
fclose($h);


var_dump(gzfile( $filename ) );

unlink($filename);
rmdir($dirname);
echo "===DONE===\n";
}
