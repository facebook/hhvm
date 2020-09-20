<?hh <<__EntryPoint>> function main(): void {
$s = "abd";
try {
  $s[0]->a += 1;
} catch(Exception $e) {
  print "\nFatal error: " . $e->getMessage();
}
}
