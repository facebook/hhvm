<?hh
<<__EntryPoint>> function main(): void {
try {
	include "inc_throw.inc";
} catch (Exception $e) {
	echo "caught exception\n";
}
}
