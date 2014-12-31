<?php

$fo = new SplFileObject(__DIR__ . "/text.txt");
$fo->setFlags(SplFileObject::SKIP_EMPTY |
              SplFileObject::DROP_NEW_LINE |
              SplFileObject::READ_AHEAD);

foreach ($fo as $key=>$val) {
  var_dump($val);
}
