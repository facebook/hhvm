<?hh
/* Prototype  : bool isset  ( mixed $var  [, mixed $var  [,  $...  ]] )
 * Description:  Determine if a variable is set and is not NULL
 */

class foo {}
<<__EntryPoint>> function main(): void {
echo "*** Testing isset() : basic functionality ***\n";

$i = 10;
$f = 10.5;
$s = "Hello";
$b = true;
$n = NULL;

echo "Test multiple scalar variables in a group\n";
var_dump(isset($i, $f, $s, $b));
var_dump(isset($i, $f, $s, $b, $n));

echo "Unset a few\n";
unset($i, $b);

echo "Test again\n";
var_dump(isset($i, $f, $s, $b));

echo "\n\nArray test:\n";
$arr = vec[];
var_dump(isset($var));
var_dump(isset($var[1]));
var_dump(isset($var, $var[1]));
echo "..now set\n";
$var = dict[];
$var[1] = 10;
var_dump(isset($var));
var_dump(isset($var[1]));
var_dump(isset($var, $var[1]));

echo "===DONE===\n";
}
