<?php

$tmp = tmpfile();

fwrite($tmp, b"12345");

echo stream_get_contents($tmp, 2, 1);
echo "\n";
echo stream_get_contents($tmp, -1);
echo "\n";
echo stream_get_contents($tmp, -1, 0);
echo "\n";
echo stream_get_contents($tmp, -1, 2);
echo "\n";
echo stream_get_contents($tmp, 0, 0);
echo "\n";
echo stream_get_contents($tmp, 1, 0);
echo "\n";
echo stream_get_contents($tmp, -1);

@unlink($tmp);

?>