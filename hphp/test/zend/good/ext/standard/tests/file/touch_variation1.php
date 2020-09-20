<?hh
<<__EntryPoint>> function main(): void {
$filename = __SystemLib\hphp_test_tmppath('touch_variation1.dat');


var_dump(touch($filename, 101));
var_dump(filemtime($filename));
var_dump(fileatime($filename));

@unlink($filename);

@unlink($filename);
var_dump(touch($filename, 100, 102));
var_dump(filemtime($filename));
var_dump(fileatime($filename));

@unlink($filename);
echo "Done\n";
}
