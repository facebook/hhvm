<?hh // strict

// T<int> and T<string> are indeed different types :)
function test<T<TX>>(T<int> $x) : T<string> {
  return $x;
}
