<?hh
<<__EntryPoint>> function main(): void {
// example 1 - Timestamp is changing
$datetime = new DateTime('2016-11-09 20:00:00', new DateTimeZone('UTC'));
var_dump($datetime->getTimestamp());
$datetime->setTimezone(new DateTimeZone('-03:00'));
$datetime->setTimezone(new DateTimeZone('-03:00'));
var_dump($datetime->getTimestamp());

// example 2 - Timestamp keeps if you use getTimestamp() before second setTimeZone() calls
$datetime = new DateTime('2016-11-09 20:00:00', new DateTimeZone('UTC'));
var_dump($datetime->getTimestamp());
$datetime->setTimezone(new DateTimeZone('-03:00'));
$datetime->getTimestamp();
$datetime->setTimezone(new DateTimeZone('-03:00'));
var_dump($datetime->getTimestamp());
}
