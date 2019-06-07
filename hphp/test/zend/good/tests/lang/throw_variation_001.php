<?hh
<<__EntryPoint>> function main() {
try {
	include "inc_throw.inc";
} catch (Exception $e) {
	echo "caught exception\n";
}
}
