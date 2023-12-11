<?hh
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL
 * Source code: ext/standard/array.c
 * Alias to functions:
 */

// define some classes
class classWithToString {
    public function __toString() :mixed{
        return "Class A object";
    }
}

class classWithoutToString { }

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
    // We're testing sort order not errors so ignore.
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_multisort() : usage variation  - test sort order of all types***\n";

set_error_handler(test_error_handler<>);

$inputs = dict[
      'int 0' => 0,
      'float -10.5' => -10.5,
      0 => 'Array',
      'uppercase NULL' => NULL,
      'lowercase true' => true,
      'empty string DQ' => "",
      'string DQ' => "string",
      'instance of classWithToString' => new classWithToString(),
      'instance of classWithoutToString' => new classWithoutToString()
];

$string = SORT_STRING;
var_dump(array_multisort2(inout $inputs, inout $string));
var_dump($inputs);

echo "===DONE===\n";
}
