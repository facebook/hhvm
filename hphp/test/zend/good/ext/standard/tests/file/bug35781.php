<?php

$filename = dirname(__FILE__)."/bug35781.txt";
	
$fp = fopen($filename, "w");
stream_filter_append($fp, "string.rot13", -49);
fwrite($fp, "This is a test\n");
rewind($fp);
fpassthru($fp);
fclose($fp);

var_dump(file_get_contents($filename));

@unlink($filename);

echo "Done\n";
?>