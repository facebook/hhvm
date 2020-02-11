<?hh <<__EntryPoint>> function main(): void {
var_dump(
    filter_var("", FILTER_DEFAULT),
    filter_var("", FILTER_DEFAULT, darray['flags' => FILTER_FLAG_EMPTY_STRING_NULL])
);
}
