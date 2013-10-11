<?php

$file = new SplFileObject(__DIR__ . '/csv.csv');
$file->setFlags(SplFileObject::READ_CSV);
$file->setCsvControl(';');

foreach ($file as $line) {
  var_dump($line);
  exit;
}
