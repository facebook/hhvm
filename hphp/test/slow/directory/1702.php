<?php

$d = dir(__DIR__."/../../sample_dir/");
echo "Path: " . $d->path . "\n";
$files = array();
 // order changes per machine
while (false !== ($entry = $d->read())) {
   $files[] = $entry."\n";
}
asort($files);
var_dump(array_values($files));

$d->rewind();
$files = array();
 // order changes per machine
while (false !== ($entry = $d->read())) {
   $files[] = $entry."\n";
}
asort($files);
var_dump(array_values($files));
$d->close();
