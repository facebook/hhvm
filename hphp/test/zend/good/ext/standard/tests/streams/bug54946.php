<?hh
<<__EntryPoint>> function main(): void {
$filename = __SystemLib\hphp_test_tmppath('phpbug');
$stream = fopen($filename, "w"); // w or a
$retval = stream_get_contents($stream, 1, 1);
fclose($stream);
var_dump($retval);
unlink($filename);



$filename = __SystemLib\hphp_test_tmppath('phpbug2');

$stream = fopen($filename, "a");
$retval = stream_get_contents($stream, 1, 1);
var_dump($retval);
fclose($stream);
unlink($filename);



$filename = __SystemLib\hphp_test_tmppath('phpbug3');

$stream = fopen($filename, "a");
fseek($stream, 1);
$retval = stream_get_contents($stream, 1);
var_dump($retval);
fclose($stream);
unlink($filename);
echo "===DONE===\n";
}
