<?hh

<<__EntryPoint>> function main(): void {
date_default_timezone_set('America/Chicago');

$date = new DateTime('2020-03-08 01:30:00');
echo $date->setTime(2, 0)->format('Y-m-d H:i:s T/e - U') . "\n";

$date = new DateTime('2020-03-08 01:30:00');
echo $date->setTime(2, 30)->format('Y-m-d H:i:s T/e - U') . "\n";

$date = new DateTime('2020-03-08 01:30:00');
echo $date->setTime(3, 0)->format('Y-m-d H:i:s T/e - U') . "\n";

$date = new DateTime('2020-03-08 01:30:00');
echo $date->setTime(1, 59, 59)->format('Y-m-d H:i:s T/e - U') . "\n";
}
