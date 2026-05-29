<?hh

// RepresentableAs<int> with correct int value should succeed
function takes_ra(HH\Runtime\RepresentableAs<int> $x): void {
  echo "got: ".(string)HH\Runtime\reveal($x)."\n";
}

function returns_ra(): HH\Runtime\RepresentableAs<int> {
  return 42;
}

<<__EntryPoint>>
function main(): void {
  takes_ra(42);
  $v = returns_ra();
  echo "returned: ".(string)HH\Runtime\reveal($v)."\n";
}
