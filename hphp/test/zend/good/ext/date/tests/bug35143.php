<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set("UTC");

var_dump(date_default_timezone_get());
var_dump(gettimeofday());
}
