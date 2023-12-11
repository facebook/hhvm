<?hh <<__EntryPoint>> function main(): void {
$foo = "ABC = DEF";
$fmt = "%s = %s %n";

/* $res_a[2] is supposed to be a integer value that
 * represents the number of characters consumed so far
 */
list($res_a0, $res_a1, $res_a2) = sscanf($foo, $fmt);

$res_b = sscanf($foo, $fmt);

var_dump(vec[$res_a0, $res_a1, $res_a2]);
var_dump($res_b);
}
