<?hh
/* Prototype  : float log1p  ( float $arg  )
 * Description: Returns log(1 + number), computed in a way that is accurate even
 *                when the value of number is close to zero
 * Source code: ext/standard/math.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing log1p() : basic functionality ***\n";

$values = vec[23,
                -23,
                2.345e1,
                -2.345e1,
                0x17,
                027,
                "23",
                "23.45",
                "2.345e1",
                null,
                true,
                false];

echo "\n LOG1p tests\n";

foreach($values as $value) {
    $value__str = (string)($value);
    echo "\n-- log1p $value__str --\n";
    var_dump(log1p((float)$value));
};
echo "===Done===";
}
