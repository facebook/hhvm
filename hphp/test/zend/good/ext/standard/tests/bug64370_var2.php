<?hh
<<__EntryPoint>> function main(): void {
$i = 0;
while(100000 > $i++) {
	$m0 = microtime(true);
	$m1 = microtime(true);
	$d = $m1 - $m0;

	/*echo "$d\n";*/

	if ($d < 0) {
		exit("failed in {$i}th iteration");
	}
}
echo "ok\n";
echo "===DONE===\n";
}
