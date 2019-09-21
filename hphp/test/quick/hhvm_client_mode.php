<?hh
<<__EntryPoint>> function main(): void {
$hhvm = PHP_BINARY;
$file = '/../../a/b/test.php';
$cmd = "$hhvm --no-config $file 2>&1";
$output = null;
$return_var = -1;
$out = exec($cmd, inout $output, inout $return_var);
echo $out;
}
