<?hh
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_multisort() : natural sorting case-sensitive\n";

$a = vec[
    'Second',
    'First.1',
    'First.2',
    'First.3',
    'Twentieth',
    'Tenth',
    'Third',
];

$b = vec[
    '2 a',
    '1 bb 1',
    '1 bB 2',
    '1 BB 3',
    '20 c',
    '10 d',
    '3 e',
];

$natural = SORT_NATURAL;
array_multisort3(inout $b, inout $natural, inout $a);

var_dump($a, $b);

echo "===DONE===\n";
}
