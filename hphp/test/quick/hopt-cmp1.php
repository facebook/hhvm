<?hh

function main($x, $y) {
  while ($x < $y) {
    echo $x++ . "\n";
  }
}
<<__EntryPoint>> function main_entry(): void {
main(1, 4);

echo "Done\n";
}
