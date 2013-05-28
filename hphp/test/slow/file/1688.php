<?php

$src = tmpfile();
$dst = tmpfile();
fwrite($src, 'a');
fseek($src, 0);
stream_copy_to_stream($src, $dst);
fseek($dst, 0);
$str = stream_get_contents($dst);
fseek($src, 0);
stream_copy_to_stream($src, $dst);
fseek($dst, 0);
$str = stream_get_contents($dst);
echo $str;
