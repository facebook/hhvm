<?php

$descriptorspec = array(
  0 => array("pipe", "r+"),
  1 => array("pipe", "w"),
  2 => array("pipe", "a"),
);

$process = proc_open('echo', $descriptorspec, $io);
var_dump(stream_set_write_buffer($io[0], 0));


$fd = fopen(__DIR__.'/stream_set_write_buffer.php.sample', 'rb');
var_dump(stream_set_write_buffer($fd, 0));
var_dump(stream_set_write_buffer($fd, 4096));
var_dump(trim(fgets($fd)));
fclose($fd);
