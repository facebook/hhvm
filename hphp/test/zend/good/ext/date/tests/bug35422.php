<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set("UTC");

echo date(DATE_ISO8601, strtotime("July 1, 2000 00:00:00 UTC")) . "\n";
echo date(DATE_ISO8601, strtotime("July 1, 2000 00:00:00 GMT")) . "\n";
}
