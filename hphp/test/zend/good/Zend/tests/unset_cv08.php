<?hh
<<__EntryPoint>> function main(): void {
$a = "ok\n";
$b = "ok\n";
@array_unique($GLOBALS['GLOBALS']);
echo $a;
echo $b;
echo "ok\n";
}
