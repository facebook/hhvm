<?hh

function with_ra_return(): HH\Runtime\RepresentableAs<int> {
  return 42;
}

function with_ra_param(HH\Runtime\RepresentableAs<int> $x): void {}

<<__EntryPoint>>
function main(): void {
  $rf = new ReflectionFunction('with_ra_return');
  echo "Return type: ".$rf->getReturnTypeText()."\n";

  $rf2 = new ReflectionFunction('with_ra_param');
  $params = $rf2->getParameters();
  echo "Param type: ".$params[0]->getTypeText()."\n";
}
