<?hh <<__EntryPoint>> function main(): void {
$ta = darray[0 => 1, 1 => 2, 2 => 3];
$ta[NULL] = "Null Value";

var_dump(array_key_exists(NULL, $ta));
}
