<?php

$input = fopen('php://memory','r+');
fwrite($input,'123456');

rewind($input);

$oneByte = fread($input, 1);

$output = fopen('php://memory','r+');
fwrite($output,$oneByte);
stream_copy_to_stream($input, $output);

rewind($output);

var_dump(stream_get_contents($output));
