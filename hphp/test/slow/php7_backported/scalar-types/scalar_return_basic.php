<?hh
class Stringable { public function __toString() { return "foobar"; } }
<<__EntryPoint>> function main(): void {
require 'fix_exceptions.inc'; fix_exceptions();
$functions = darray[
    'int' => function ($i): int { return $i; },
    'float' => function ($f): float { return $f; },
    'string' => function ($s): string { return $s; },
    'bool' => function ($b): bool { return $b; }
];

$values = varray[
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
    varray[],
    new StdClass,
    new Stringable,
    fopen("data:text/plain,foobar", "r")
];

foreach ($functions as $type => $function) {
    echo PHP_EOL, "Testing '$type' typehint:", PHP_EOL;
    foreach ($values as $value) {
        echo "*** Trying ";
        var_dump($value);
        try {
            var_dump($function($value));
        } catch (TypeError $e) {
            echo "*** Caught " . $e->getMessage() . PHP_EOL;
        }
    }
}

echo PHP_EOL . "Done";
}
