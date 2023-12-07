<?hh

// This doesn't work for windows, time, atime usage results in very different
// output to linux. This could be a php.net bug on windows or a windows querk.
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'touch.dat';
try { var_dump(touch()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(touch($filename));
var_dump(filemtime($filename));
unlink($filename);
var_dump(touch($filename, 101));
var_dump(filemtime($filename));

unlink($filename);
var_dump(touch($filename, -1));
var_dump(filemtime($filename));

unlink($filename);
var_dump(touch($filename, 100, 100));
var_dump(filemtime($filename));

unlink($filename);
var_dump(touch($filename, 100, -100));
var_dump(filemtime($filename));

var_dump(touch("/no/such/file/or/directory"));

unlink($filename);

echo "Done\n";
}
