<?hh // strict

function main(mixed $x): void {
  if ($x is bool) {
    echo "bool\n";
  }
  if ($x is int) {
    echo "int\n";
  }
  if ($x is float) {
    echo "float\n";
  }
  if ($x is string) {
    echo "string\n";
  }
  if ($x is resource) {
    echo "resource\n";
  }
  if ($x is vec) {
    echo "vec\n";
  }
  if ($x is dict) {
    echo "dict\n";
  }
  if ($x is keyset) {
    echo "keyset\n";
  }
}

main(true);
main(0);
main(1.5);
main("foo");
main(STDIN);
main(vec[]);
main(dict[]);
main(keyset[]);
