<?hh <<__EntryPoint>> function main(): void {
var_dump( timezone_name_from_abbr("", (int)(5.5*3600), 0) );
var_dump( timezone_name_from_abbr("", 28800, 0) );
}
