<?hh
/* Prototype  : array class_uses(mixed what [, bool autoload ])
 * Description: Return all traits used by a class
 * Source code: ext/spl/php_spl.c
 * Alias to functions:
 */
class fs {}
<<__EntryPoint>> function main(): void {
echo "*** Testing class_uses() : variation ***\n";

echo "--- testing no traits ---\n";
var_dump(class_uses(new fs));
var_dump(class_uses('fs'));

echo "\n--- testing autoload ---\n";
var_dump(class_uses('non_existent'));
var_dump(class_uses('non_existent2', false));

echo "===DONE===\n";
}
