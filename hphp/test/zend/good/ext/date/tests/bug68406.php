<?hh
<<__EntryPoint>> function main(): void {


$tz1 = new DateTimeZone('Europe/Berlin');
$tz2 = new DateTimeZone('Europe/Berlin');

$d = new DateTime('2014-12-24 13:00:00', $tz1);
var_dump($d->getTimezone(), $tz2);

if($tz2 == $d->getTimezone()) {
    echo "yes";
}
else {
    echo "no";
}
}
