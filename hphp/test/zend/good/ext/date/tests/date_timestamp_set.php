<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');
$dftz021 = date_default_timezone_get(); //UTC

$dtms021 = date_create();

date_timestamp_set($dtms021, 1234567890);

var_dump(date_format($dtms021, 'B => (U) => T Y-M-d H:i:s'));
}
