<?hh

<<__Memoize>>
function memo($x) {
  echo "ok\n";
}

function foo() {
  echo "1\n";
  memo(2);
  echo "2\n";
  memo(2);
  echo "3\n";
}

<<__EntryPoint>>
function main() {
  HH\ImplicitContext\soft_run_with(foo<>, "SOFT_KEY");
}
