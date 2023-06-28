<?hh
function separate_zval(inout $var) :mixed{ }
<<__EntryPoint>> function main(): void {
$s = fopen(__FILE__, "rb"); $s2 = $s;
separate_zval(inout $s2);
fclose($s);
echo fread($s2, strlen("<?hh"));
echo "\nDone.\n";
}
