<?php

$input = fopen('php://memory','r+');

$str = 'Hello, World!';
for ($i = 0; $i < 16; $i++) {
  $str .= $str;
}


fwrite($input, $str);

rewind($input);

$output = fopen('php://memory','r+');
stream_copy_to_stream($input, $output);

rewind($output);

echo stream_get_contents($output) . "\n";
