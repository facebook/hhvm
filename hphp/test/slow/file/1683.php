<?php

$fh = fopen('php://output', 'w');
if (!$fh) {
  throw new Exception('foo');
}
fprintf($fh, "hello\n");
var_dump(fflush($fh));
var_dump(fclose($fh));
$fh = fopen('php://output', 'a');
if (!$fh) {
  throw new Exception('foo');
}
fprintf($fh, "hello\n");
var_dump(fflush($fh));
var_dump(fclose($fh));
$fh = fopen('php://output', 'r');
if (!$fh) {
  throw new Exception('foo');
}
fprintf($fh, "hello\n");
var_dump(fflush($fh));
var_dump(fclose($fh));
