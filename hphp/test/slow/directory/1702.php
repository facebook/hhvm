<?php

require 'test/sample_dir/fix_mtimes.inc';

$d = dir("test/sample_dir/");
echo "Path: " . $d->path . "\n";
while (false !== ($entry = $d->read())) {
   echo $entry."\n";
}
$d->rewind();
while (false !== ($entry = $d->read())) {
   echo $entry."\n";
}
$d->close();
