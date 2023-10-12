<?hh // strict

function test(bool $b, arraykey $x): void {
  while ($b) {
    if ($x === 's') {
      print("x was s\n");
    }
    $x = 5;
  }
  print("$x\n");
}
// Avoid lint error about file defining only one function
function dummy() {}
