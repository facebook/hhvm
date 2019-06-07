<?hh
<<__EntryPoint>> function main() {
try {
	$x = new splqueue;
	$x->offsetSet(0, 0);
} catch (Exception $e) { }
echo "DONE";
}
