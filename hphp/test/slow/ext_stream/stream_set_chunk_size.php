<?php

$fd = fopen(__DIR__.'/stream_set_chunk_size.php.sample', 'rb');
var_dump(stream_set_chunk_size($fd, 256));
fclose($fd);
