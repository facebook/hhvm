<?hh
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
chdir(__SystemLib\hphp_test_tmproot());
$filename = "bug51997.bz2";
$str = "This is a test string.\n";
$bz = bzopen($filename, "w");
bzwrite($bz, $str);
bzclose($bz);

$bz = bzopen($filename, "r");
fseek($bz, 0, SEEK_CUR);
print bzread($bz, 10);
print bzread($bz);
bzclose($bz);
unlink($filename);
}
