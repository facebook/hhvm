<?php

$d = dir(__DIR__);
while ($ent = readdir($d->handle)) {
  if ($ent == 'test_ext_file.txt') {
    var_dump($ent);
  }
}
closedir($d);

$d = opendir(__DIR__);
while ($ent = readdir($d)) {
  if ($ent == 'test_ext_file.txt') {
    var_dump($ent);
  }
}

rewinddir($d);

while ($ent = readdir($d)) {
  if ($ent == 'test_ext_file.txt') {
    var_dump($ent);
  }
}

closedir($d);

foreach (scandir(__DIR__) as $x) {
  if ($x == 'test_ext_file.txt') {
    var_dump($x);
  }
}

