<?hh <<__EntryPoint>> function main() {
var_dump(filter_var(new stdClass, FILTER_VALIDATE_EMAIL));
}
