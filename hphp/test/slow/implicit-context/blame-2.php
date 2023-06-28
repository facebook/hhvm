<?hh

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_ic_inaccessible($x) :mixed{
  echo "ok\n";
}

function foo() :mixed{
  echo "1\n";
  soft_ic_inaccessible(2);
  echo "2\n";
  soft_ic_inaccessible(2);
  echo "3\n";
}

<<__EntryPoint>>
function main() :mixed{
  HH\ImplicitContext\soft_run_with(foo<>, "SOFT_KEY");
}
