<?hh
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'phpbug';
$stream = fopen($filename, "w"); // w or a
$retval = stream_get_contents($stream, 1, 1);
fclose($stream);
var_dump($retval);
unlink($filename);



$filename = sys_get_temp_dir().'/'.'phpbug2';

$stream = fopen($filename, "a");
$retval = stream_get_contents($stream, 1, 1);
var_dump($retval);
fclose($stream);
unlink($filename);



$filename = sys_get_temp_dir().'/'.'phpbug3';

$stream = fopen($filename, "a");
fseek($stream, 1);
$retval = stream_get_contents($stream, 1);
var_dump($retval);
fclose($stream);
unlink($filename);
echo "===DONE===\n";
}
