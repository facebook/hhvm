<?hh
<<__EntryPoint>> function main(): void {
$fn = __SystemLib\hphp_test_tmppath('test.tmp');
$fh = fopen($fn, 'xb');
$ch = curl_init('http://www.yahoo.com/');
var_dump(curl_setopt($ch, CURLOPT_FILE, $fh));
echo "Done.\n";

unlink($fn);
}
