<?php
$iToday = new DateTimeImmutable('today');
$iTomorrow = new DateTimeImmutable('tomorrow');

$mToday = new DateTime('today');
$mTomorrow = new DateTime('tomorrow');

var_dump($iToday < $iTomorrow);
var_dump($iToday == $iTomorrow);
var_dump($iToday > $iTomorrow);

var_dump($iToday == $mToday);
var_dump($iToday === $mToday);

var_dump($iToday < $mTomorrow);
var_dump($iToday == $mTomorrow);
var_dump($iToday > $mTomorrow);
?>