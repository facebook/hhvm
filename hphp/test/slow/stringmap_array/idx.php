<?hh

function main() {
  $a = msarray();
  idx($a, "no warning");
  idx($a, 10); // warning
}

main();
