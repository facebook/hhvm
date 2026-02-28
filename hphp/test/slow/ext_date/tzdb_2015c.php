<?hh


<<__EntryPoint>>
function main_tzdb_2015c() :mixed{
date_default_timezone_set('America/Santiago');
var_dump(strtotime('April 26 2015, 00:00:00'));
}
