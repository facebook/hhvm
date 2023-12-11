<?hh <<__EntryPoint>> function main(): void {
var_dump(
    filter_var("", FILTER_DEFAULT),
    filter_var("", FILTER_DEFAULT, dict['flags' => FILTER_FLAG_EMPTY_STRING_NULL])
);
}
