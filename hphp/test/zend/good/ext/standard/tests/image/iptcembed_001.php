<?hh
<<__EntryPoint>> function main(): void {
$file = __SystemLib\hphp_test_tmppath('iptcembed_001.data');
$fp = fopen($file, "w");
fwrite($fp, "-1-1");
fclose($fp);

var_dump(iptcembed('-1', $file, -1));
unlink($file);

echo "Done\n";
}
