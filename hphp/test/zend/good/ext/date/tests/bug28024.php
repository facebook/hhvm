<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('Europe/Berlin');
echo strtotime("17:00 2004-01-01"), "\n";
echo date("Y-m-d H:i:s T", strtotime("17:00 2004-01-01"));
}
