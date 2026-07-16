<?hh

function add_value(int $x, int $y = 123456): int {
  return $x + $y;
}

<<__EntryPoint>> function main(): void {
error_reporting(-1);

echo "================= xxx =================\n";

// numeric literals with underscores

var_dump(1234567890);
var_dump(0xffcc1234);
var_dump(0b11001100);
var_dump(0666555444);
var_dump(123.456);
var_dump(123e-456);
var_dump(.123e-456);
var_dump(0.1234e+25);
var_dump(0e-15);
var_dump(0341.5136);
var_dump(add_value(1));

$refFunction = new ReflectionFunction('add_value');
$parameters = $refFunction->getParameters();
$yParameter = $parameters[1];
var_dump($yParameter->getDefaultValue());
}
