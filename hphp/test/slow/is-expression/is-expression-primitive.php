<?hh

function main(mixed $x): void {
  if ($x is bool) {
    echo "bool\n";
  } else if ($x is int) {
    echo "int\n";
  } else if ($x is float) {
    echo "float\n";
  } else if ($x is string) {
    echo "string\n";
  } else if ($x is resource) {
    echo "resource\n";
  }
}


<<__EntryPoint>>
function main_is_expression_primitive() :mixed{
main(true);
main(0);
main(1.5);
main("foo");
main(fopen(__FILE__, 'r'));
}
