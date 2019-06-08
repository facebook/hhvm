<?hh
<<__EntryPoint>> function main(): void {
$hhvm = PHP_BINARY;
$file = '/../../a/b/test.php';
$cmd = "$hhvm --no-config $file 2>&1";
$out = exec($cmd);
echo $out;
}
