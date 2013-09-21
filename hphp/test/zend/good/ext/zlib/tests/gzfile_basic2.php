<?php
$plaintxt = b<<<EOT
hello world
is a very common test
for all languages
EOT;
$dirname = 'gzfile_temp';
$filename = $dirname.'/gzfile_basic2.txt';
mkdir($dirname);
$h = fopen($filename, 'w');
fwrite($h, $plaintxt);
fclose($h);


var_dump(gzfile( $filename ) );

unlink($filename);
rmdir($dirname);
?>
===DONE===