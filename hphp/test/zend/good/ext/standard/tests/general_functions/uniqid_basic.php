<?hh
/* Prototype  : string uniqid  ([ string $prefix= ""  [, bool $more_entropy= false  ]] )
 * Description: Gets a prefixed unique identifier based on the current time in microseconds.
 * Source code: ext/standard/uniqid.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing uniqid() : basic functionality ***\n";
echo "\nuniqid() without a prefix\n";
var_dump(uniqid());
var_dump(uniqid('', true));
var_dump(uniqid('', false));
echo "\n\n";

echo "uniqid() with a prefix\n";

// Use a fixed prefix so we can ensure length of o/p id is fixed
$prefix = vec[
                99999,
                "99999",
                10.5e2,
                null,
                true,
                false
                ];

for ($i = 0; $i < count($prefix); $i++) {
    var_dump(uniqid((string)$prefix[$i]));
    var_dump(uniqid((string)$prefix[$i], true));
    var_dump(uniqid((string)$prefix[$i], false));
    echo "\n";
}

echo "===DONE===\n";
}
