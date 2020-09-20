<?hh
/* This works */
<<__EntryPoint>> function main(): void {
$f = darray['7' => 0];
var_dump($f);
var_dump(array_key_exists(7, $f));
var_dump(array_key_exists('7', $f));

print "----------\n";
/* This doesn't */
$f = array_flip(varray['7']);
var_dump($f);
var_dump(array_key_exists(7, $f));
var_dump(array_key_exists('7', $f));
}
