<?hh

function main() {
  $a = miarray();
  $a[] = 1; // Warns
  $a[] = 2; // Doesn't warn
  var_dump($a);
}

main();
