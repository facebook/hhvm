<?php

$resource = fopen(dirname(__FILE__).'/bug30362.txt', 'rb');

for ($i = 0; ($i < 10) && !feof($resource); ++$i ) {
    $a = "Y";
    $line = stream_get_line($resource, 50, $a);
    echo $line . "\n";
}
fclose($resource);
        
?>