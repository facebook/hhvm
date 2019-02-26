<?php
function flock_or_die($filename, $resource, $flock_op) {
  $r = flock($resource, $flock_op);
  var_dump($r);
 }


<<__EntryPoint>>
function main_1689() {
$filename = '/tmp/flock_file.dat';
$resource = fopen($filename, 'w');
flock_or_die($filename, $resource, LOCK_EX);
flock_or_die($filename, $resource, LOCK_UN);
unlink($filename);
}
