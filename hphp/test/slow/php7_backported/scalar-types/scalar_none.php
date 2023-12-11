<?hh


<<__EntryPoint>>
function main_scalar_none() :mixed{
require 'fix_exceptions.inc';
fix_exceptions();

$functions = dict[
    'int' => function (int $i) { return $i; },
    'float' => function (float $f) { return $f; },
    'string' => function (string $s) { return $s; },
    'bool' => function (bool $b) { return $b; },
    'int nullable' => function (?int $i = NULL) { return $i; },
    'float nullable' => function (?float $f = NULL) { return $f; },
    'string nullable' => function (?string $s = NULL) { return $s; },
    'bool nullable' => function (?bool $b = NULL) { return $b; }
];

foreach ($functions as $type => $function) {
    echo "Testing $type:", PHP_EOL;
    try {
        var_dump($function(null));
    } catch (TypeError $e) {
        echo "*** Caught " . $e->getMessage() . PHP_EOL;
    }
}
echo PHP_EOL . "Done";
}
