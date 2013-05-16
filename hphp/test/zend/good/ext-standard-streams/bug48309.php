<?php

$tmp = tmpfile();
fwrite($tmp, b'test');
fseek($tmp, 0, SEEK_SET);

echo "-- stream_copy_to_stream() --\n";

fseek($tmp, 0, SEEK_SET);
stream_copy_to_stream($tmp, STDOUT, 2);

echo "\n";
var_dump(stream_get_contents($tmp));

echo "-- fpassthru() --\n";

fseek($tmp, 0, SEEK_SET);
fpassthru($tmp);

echo "\n";
var_dump(stream_get_contents($tmp));

?>