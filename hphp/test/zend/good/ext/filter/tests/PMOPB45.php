<?hh <<__EntryPoint>> function main(): void {
$var = "test@example.com\n";
var_dump(filter_var($var, FILTER_VALIDATE_EMAIL));
}
