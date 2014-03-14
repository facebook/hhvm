<?php
$dateTime = \DateTime::createFromFormat('U', 1293839999,
  new \DateTimeZone('GMT'));
$var1 = $dateTime->format('T');
$var2 = $dateTime->format('e');
var_dump($var1);
var_dump($var2);
