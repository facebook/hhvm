<?hh

function A_foo($arg) {
  echo "In A_foo()\n";
  if (is_string($arg)) {
    echo "  Got string!\n";
  } else if (is_bool($arg)) {
    echo "  Got bool!\n";
  } else if (is_int($arg)) {
    echo "  Got int!\n";
  }
}
