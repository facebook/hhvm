<?hh

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_ic_inaccessible($x) {
  echo "ok\n";
}

function foo() {
  echo "1\n";
  soft_ic_inaccessible(2);
  echo "2\n";
  soft_ic_inaccessible(2);
  echo "3\n";
}

<<__EntryPoint>>
function main() {
  HH\ImplicitContext\soft_run_with(foo<>, "SOFT_KEY");
}
