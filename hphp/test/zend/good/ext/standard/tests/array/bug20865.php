<?hh <<__EntryPoint>> function main(): void {
$ta = varray[1, 2, 3];
$ta[NULL] = "Null Value";

var_dump(array_key_exists(NULL, $ta));
}
