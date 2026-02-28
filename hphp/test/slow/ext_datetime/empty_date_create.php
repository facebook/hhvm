<?hh

<<__EntryPoint>>
function main_empty_date_create() :mixed{
date_default_timezone_set("GMT");
var_dump(date_create());
var_dump(date_create(""));
var_dump(date_create(null));
var_dump(date_create(null, timezone_open("UTC")));
}
