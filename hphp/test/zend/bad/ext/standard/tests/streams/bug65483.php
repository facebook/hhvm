<?php

$data = 'a b=c d';

$fd = fopen('php://temp', 'w+');
fwrite($fd, $data);
rewind($fd);

$res = stream_filter_append($fd, 'convert.quoted-printable-encode', STREAM_FILTER_READ);
var_dump(stream_get_contents($fd, -1, 0));

fclose($fd);

?>