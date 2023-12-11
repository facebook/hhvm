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
<<__EntryPoint>> function main(): void {
echo "\n\nArray containing same object twice:\n";
$obj = new stdClass;
$a = dict[];
$a[0] = $obj;
$a[1] = $a[0];
var_dump($a);

$ser = serialize($a);
var_dump($ser);

$ua = unserialize($ser);
var_dump($ua);
$ua[0]->a = "newProp";
var_dump($ua);
$ua[0] = "a0.changed";
var_dump($ua);

echo "\n\nObject containing same object twice:";
$obj = new stdClass;
$contaner = new stdClass;
$contaner->a = $obj;
$contaner->b = $contaner->a;
var_dump($contaner);

$ser = serialize($contaner);
var_dump($ser);

$ucontainer = unserialize($ser);
var_dump($ucontainer);
$ucontainer->a->a = "newProp";
var_dump($ucontainer);
$ucontainer->a = "container->a.changed";
var_dump($ucontainer);

echo "Done";
}
