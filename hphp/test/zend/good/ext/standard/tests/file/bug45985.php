<?hh
<<__EntryPoint>> function main(): void {
$file = __SystemLib\hphp_test_tmppath('__tmp_35740.dat');
file_put_contents($file, 'test');
$f = fopen($file, 'r');
touch($file);
fclose($f);
@unlink($file);
echo "ok";
}
