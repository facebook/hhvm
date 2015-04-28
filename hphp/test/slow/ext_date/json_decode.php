<?php

$dateTimeImmutable = new DateTimeImmutable('2015-05-03', new DateTimeZone('Arctic/Longyearbyen'));
var_dump(json_encode($dateTimeImmutable));
