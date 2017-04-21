<?hh // strinc

function test(): void {
  $x = 42;
  switch ($x) {
  case 0:
    echo "zero";
    break;
  default:
    echo "default";
    break;
  }
}
