<?hh

<<__EntryPoint>>
function main_mb_ereg_search_crash() :mixed{
error_reporting(0);
var_dump(mb_ereg_search('abc', ''));
}
