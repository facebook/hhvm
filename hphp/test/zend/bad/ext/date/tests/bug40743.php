<?php
$dt = new DateTime('@1200506699', new DateTimeZone('Europe/Berlin'));
echo $dt->format(DATE_RFC822), "\n";
echo $dt->format('T e Z'), "\n";
echo "-----\n";

date_default_timezone_set('America/New_York');

$dt = new DateTime('16 Jan 08 13:04:59');
echo $dt->format(DATE_RFC822 . " e T O U"), "\n";

$dt = new DateTime('@1200506699');
echo $dt->format(DATE_RFC822 . " e T O U"), "\n";

$dt = new DateTime('@1200506699');
$dt->setTimezone( new DateTimeZone( 'America/New_York' ) );
echo $dt->format(DATE_RFC822 . " e T O U"), "\n";

$dt = new DateTime('@1200506699', new DateTimeZone('Europe/Berlin'));
echo $dt->format(DATE_RFC822 . " e T O U"), "\n";

$dt = new DateTime('16 Jan 08 13:04:59 America/Chicago');
echo $dt->format(DATE_RFC822 . " e T O U"), "\n";

$dt = new DateTime('16 Jan 08 13:04:59 America/Chicago', new DateTimeZone('Europe/Berlin'));
echo $dt->format(DATE_RFC822 . " e T O U"), "\n";
?>