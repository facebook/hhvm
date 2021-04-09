<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');
$date = '2016 12:00:00 15';
$format = 'Y H:i:s z';
var_dump(DateTime::createFromFormat($format, $date));

$date = '16 12:00:00 2016';
$format = 'z H:i:s Y';
var_dump(DateTime::createFromFormat($format, $date));
}
