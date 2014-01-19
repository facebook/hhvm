<?php

$descriptorspec = array(
  0 => array("pipe", "r+"),
  1 => array("pipe", "w"),
  2 => array("pipe", "a"),
);

$process = proc_open('echo', $descriptorspec, $io);
var_dump(stream_set_write_buffer($io[0], 0));
