<?php

$fo = new SplFileObject(__DIR__.'/csv.csv');
$fo->setFlags(SplFileObject::SKIP_EMPTY |
              SplFileObject::DROP_NEW_LINE |
              SplFileObject::READ_AHEAD |
              SplFileObject::READ_CSV);
$fo->setCsvControl(';');

foreach ($fo as $key=>$val) {
  var_dump($val);
}
