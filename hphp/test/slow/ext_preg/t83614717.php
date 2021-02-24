<?hh

<<__EntryPoint>>
function main(): void {
  $multiplier = 1073741824;
  $s = str_repeat("a", $multiplier);
  preg_quote($s);
  echo "FAIL!\n";
}
