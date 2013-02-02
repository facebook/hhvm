<?

function woot(string $val = D00Dz) {
  echo $val;
}

function main() {
  woot();
  // Variants are ok, too.
  $s = "m00t";
  $s[0] = "w";
  $t = &$s;
  woot($t);
  define('D00Dz', 123);
  woot();
}

main();

