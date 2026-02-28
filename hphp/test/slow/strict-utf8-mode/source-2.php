<?hh

<<__EntryPoint>>
function main(): void {
  // Purposely test a non-utf8 string
  $a = "aÿc\n";
  echo "done\n";
}
