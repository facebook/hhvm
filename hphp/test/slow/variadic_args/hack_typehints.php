<?hh

// We are allowing type hints on variadic function for Hack modes for the type
// checker, even though we will not check that every element passed into the
// variadic matches the type hint. This defers from the PHP behavior.

function variadic_hinted_scalars(int ...$objects) :mixed{
  var_dump($objects);
}

function main() :mixed{
  variadic_hinted_scalars(1, 2, 3, 4, true);
}

<<__EntryPoint>>
function main_hack_typehints() :mixed{
main();

$rf = new ReflectionFunction('variadic_hinted_scalars');
$rps = $rf->getParameters();
$rp = $rps[0];

echo 'is_variadic = ' . (string)($rp->isVariadic()) . PHP_EOL;
echo 'name = ' . $rp->getName() . PHP_EOL;
echo 'type = ' . $rp->getTypeText() . PHP_EOL;
}
