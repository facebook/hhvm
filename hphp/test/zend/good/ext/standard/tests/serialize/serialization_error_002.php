<?hh
/* Prototype  : proto string serialize(mixed variable)
 * Description: Returns a string representation of variable (which can later be unserialized)
 * Source code: ext/standard/var.c
 * Alias to functions:
 */
/* Prototype  : proto mixed unserialize(string variable_representation)
 * Description: Takes a string representation of variable and recreates it
 * Source code: ext/standard/var.c
 * Alias to functions:
 */

// candidate safe idiom for determining whether data is serialized
function isSerialized($str) :mixed{
    return ($str == serialize(false) || @unserialize($str) !== false);
}
<<__EntryPoint>> function main(): void {
echo "*** Testing unserialize() error/boolean distinction ***\n";

$garbage = "obvious non-serialized data";
$serialized_false = serialize(false);

var_dump($serialized_false);

$deserialized_garbage = unserialize($garbage);
var_dump($deserialized_garbage);

$deserialized_false = unserialize($serialized_false);
var_dump($deserialized_false);

echo "unserialize error and deserialized false are identical? " . (string)((bool) ($deserialized_false == $deserialized_garbage)) . "\n";

// Test unserialize error idiom
var_dump(isSerialized($garbage));
var_dump(isSerialized($serialized_false));

echo "Done";
}
