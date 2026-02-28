<?hh
/* Prototype  :  string number_format  ( float $number  [, int $decimals  ] )
 *               string number_format ( float $number , int $decimals , string $dec_point , string $thousands_sep )
 * Description: Format a number with grouped thousands
 * Source code: ext/standard/string.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing number_format() : basic functionality ***\n";

$values = vec[1234.5678,
                -1234.5678,
                1234.6578e4,
                -1234.56789e4];

echo "\n-- number_format tests.....default --\n";
for ($i = 0; $i < count($values); $i++) {
    try { var_dump(number_format($values[$i])); } catch (Exception $e) { var_dump($e->getMessage()); }
}

echo "\n-- number_format tests.....with two dp --\n";
for ($i = 0; $i < count($values); $i++) {
    try { var_dump(number_format($values[$i], 2)); } catch (Exception $e) { var_dump($e->getMessage()); }
}

echo "\n-- number_format tests.....English format --\n";
for ($i = 0; $i < count($values); $i++) {
    try { var_dump(number_format($values[$i], 2, '.', ' ')); } catch (Exception $e) { var_dump($e->getMessage()); }
}

echo "\n-- number_format tests.....French format --\n";
for ($i = 0; $i < count($values); $i++) {
    try { var_dump(number_format($values[$i], 2, ',' , ' ')); } catch (Exception $e) { var_dump($e->getMessage()); }
}
echo "===DONE===\n";
}
