<?hh
<<__EntryPoint>> function main(): void {
$a = "foo";

list($a, $b, $c) = $a;

var_dump($a, $b, $c);

print "----\n";

$a = new stdClass;

try {
  list($a, $b, $c) = $a;
} catch (Exception $e) {
  print "\nFatal error: " . $e->getMessage();
}
}
