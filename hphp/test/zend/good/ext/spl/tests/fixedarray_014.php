<?hh
<<__EntryPoint>> function main(): void {
try {
	$a = new SplFixedArray(NULL);
	echo $a[0]++;
} catch (Exception $e) {
	echo $e->getMessage();
}
}
