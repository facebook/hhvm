<?hh

function main() {
  $a = miarray();
  $a[10] = 10;
  array_shift($a);
}

main();
