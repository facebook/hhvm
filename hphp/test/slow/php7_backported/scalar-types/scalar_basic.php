<?hh

class Stringable {
    public function __toString()[] :mixed{
        return "foobar";
    }
}

<<__EntryPoint>> function main(): void {
require 'fix_exceptions.inc'; fix_exceptions();
$errnames = dict[
    E_NOTICE => 'E_NOTICE',
    E_WARNING => 'E_WARNING',
];

$functions = dict[
    'int' => function (int $i) { return $i; },
    'float' => function (float $f) { return $f; },
    'string' => function (string $s) { return $s; },
    'bool' => function (bool $b) { return $b; }
];

$values = vec[
    1,
    "1",
    1.0,
    1.5,
    "1a",
    "a",
    "",
    PHP_INT_MAX,
    NAN,
    TRUE,
    FALSE,
    NULL,
    vec[],
    new stdClass,
    new Stringable,
    fopen("data:text/plain,foobar", "r")
];

foreach ($functions as $type => $function) {
    echo PHP_EOL, "Testing '$type' typehint:", PHP_EOL;
    foreach ($values as $value) {
        echo PHP_EOL . "*** Trying ";
        var_dump($value);
        try {
            var_dump($function($value));
        } catch (\TypeError $e) {
            echo "*** Caught " . $e->getMessage() . PHP_EOL;
        }
    }
}
echo PHP_EOL . "Done";
}
