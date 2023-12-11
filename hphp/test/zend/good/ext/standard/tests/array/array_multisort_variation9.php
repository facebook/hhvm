<?hh
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL
 * Source code: ext/standard/array.c
 * Alias to functions:
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing array_multisort() : usage variation  - test sort order of all types***\n";

$inputs = dict[
      'int 0' => 0,
      'float -10.5' => -10.5,
      0 => vec[],
      'uppercase NULL' => NULL,
      'lowercase true' => true,
      'empty string DQ' => "",
      'string DQ' => "string",
];

$numeric = SORT_NUMERIC;
var_dump(array_multisort2(inout $inputs, inout $numeric));
var_dump($inputs);

echo "===DONE===\n";
}
