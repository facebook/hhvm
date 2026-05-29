<?hh

// RepresentableAs<int> parameter should enforce 'int' at runtime
function takes_ra(HH\Runtime\RepresentableAs<int> $x): void {
  echo "got: ".(string)$x."\n";
}

<<__EntryPoint>>
function main(): void {
  takes_ra(HH\FIXME\UNSAFE_CAST<string, HH\Runtime\RepresentableAs<int>>("hello"));
}
