<?hh
<<__EntryPoint>> function main(): void {
$foo = "50";
try {
  $foo[0]->bar = "xyz";
} catch(Exception $e) {
  print "\nFatal error: " . $e->getMessage();
}
echo "Done\n";
}
