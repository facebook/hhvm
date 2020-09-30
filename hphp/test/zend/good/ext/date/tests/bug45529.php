<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('Europe/Oslo');
$tz1 = new DateTimeZone('UTC');
$tz2 = date_create('UTC')->getTimezone();
echo $tz1->getName(), PHP_EOL;
echo $tz2->getName(), PHP_EOL;
$d = new DateTime('2008-01-01 12:00:00+0200');
$d->setTimezone($tz1);
echo $d->format(DATE_ISO8601), PHP_EOL;
$d = new DateTime('2008-01-01 12:00:00+0200');
$d->setTimezone($tz2);
echo $d->format(DATE_ISO8601), PHP_EOL;
}
