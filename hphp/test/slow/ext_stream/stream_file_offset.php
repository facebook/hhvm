<?php

$fd = fopen(__DIR__.'/stream_file_offset.php.sample', 'rb');

// fgets moves the read position.
var_dump(trim(fgets($fd)));

// when no offset is given, use read position.
var_dump(trim(stream_get_contents($fd)));

// otherwise just use the offset.
var_dump(trim(stream_get_contents($fd, -1, 5)));

fclose($fd);
