<?hh



<<__EntryPoint>>
function main_readdir() :mixed{
$d = dir(__DIR__);
$ent = readdir($d->handle);
while ($ent) {
  if ($ent == 'test_ext_file.txt') {
    var_dump($ent);
  }
  $ent = readdir($d->handle);
}
closedir($d->handle);

$d = opendir(__DIR__);
$ent = readdir($d);
while ($ent) {
  if ($ent == 'test_ext_file.txt') {
    var_dump($ent);
  }
  $ent = readdir($d);
}

rewinddir($d);

$ent = readdir($d);
while ($ent) {
  if ($ent == 'test_ext_file.txt') {
    var_dump($ent);
  }
  $ent = readdir($d);
}

closedir($d);

foreach (scandir(__DIR__) as $x) {
  if ($x == 'test_ext_file.txt') {
    var_dump($x);
  }
}
}
