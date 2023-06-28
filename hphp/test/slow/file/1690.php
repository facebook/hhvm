<?hh


<<__EntryPoint>>
function main_1690() :mixed{
$h = popen("echo foo;
 exit 2", 'r');
$content = stream_get_contents($h);
$result = pclose($h);
echo trim($content)."/".$result."/".gettype($result)."\n";
}
