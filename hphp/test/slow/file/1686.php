<?php

$name = tempnam(sys_get_temp_dir(), '1682');
$input = fopen($name, 'w+');
fwrite($input, 'hello world');
fseek($input, 0);
$output = fopen('php://memory', 'w+');
stream_copy_to_stream($input, $output);
fseek($output, 0);
$bytes = fread($output, 1024);
print "From file, without Maxlen: <".serialize($bytes).">.\n";
fseek($input, 0);
$output = fopen('php://memory', 'w+');
stream_copy_to_stream($input, $output, null);
fseek($output, 0);
$bytes = fread($output, 1024);
print "From file, using Maxlen null: <".serialize($bytes).">.\n";
fseek($input, 0);
$output = fopen('php://memory', 'w+');
stream_copy_to_stream($input, $output, -1);
fseek($output, 0);
$bytes = fread($output, 1024);
print "From file, using Maxlen -1: <".serialize($bytes).">.\n";
unlink($name);
