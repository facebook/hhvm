<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');
$interval = new DateInterval('P1D');

$dt = new DateTime('first day of January 2011');
var_dump($dt);

$dt->add($interval);
var_dump($dt);

$dt = new DateTime('first day of January 2011');

$dt->sub($interval);
var_dump($dt);
}
