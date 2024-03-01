<?hh


<<__EntryPoint>>
function main_ini_timezone() :mixed{
date_default_timezone_set('Europe/London');
var_dump(ini_get('date.timezone'));
var_dump(date_default_timezone_get());
ini_set('date.timezone', 'Europe/Paris');
var_dump(ini_get('date.timezone'));
var_dump(date_default_timezone_get());
}
