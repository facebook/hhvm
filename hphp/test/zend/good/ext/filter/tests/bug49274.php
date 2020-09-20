<?hh <<__EntryPoint>> function main(): void {
var_dump(filter_var(new stdClass, FILTER_VALIDATE_EMAIL));
}
