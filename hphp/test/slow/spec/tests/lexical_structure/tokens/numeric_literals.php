<?hh

function add_value(int $x, int $y = 123_456): int {
  return $x + $y;
}

<<__EntryPoint>> function main(): void {
error_reporting(-1);

echo "================= xxx =================\n";

// numeric literals with underscores

var_dump(1_234_567_890);
var_dump(0xff_cc_123_4);
var_dump(0b11_00_11_00);
var_dump(0666_555_444);
var_dump(1_2_3.4_5_6);
var_dump(1_2_3e-4_5_6);
var_dump(.1_2_3e-4_5_6);
var_dump(0.1_2_3_4e+2_5);
var_dump(0e-1_5);
var_dump(03_41.51_36);
var_dump(add_value(1));

$refFunction = new ReflectionFunction('add_value');
$parameters = $refFunction->getParameters();
$yParameter = $parameters[1];
var_dump($yParameter->getDefaultValue());
}
