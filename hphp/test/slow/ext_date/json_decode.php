<?php

$dateTimeZone = new DateTimeZone('Arctic/Longyearbyen');
$dateTimeImmutable = new DateTimeImmutable('2015-05-03', $dateTimeZone);
var_dump(json_encode($dateTimeImmutable));
