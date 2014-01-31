<?php
$plaintxt = b<<<EOT
hello world
is a very common test
for all languages

EOT;
$dirname = 'readgzfile_temp';
$filename = $dirname.'/readgzfile_basic.txt.gz';
mkdir($dirname);
$h = gzopen($filename, 'w');
gzwrite($h, $plaintxt);
gzclose($h);


var_dump(readgzfile( $filename ) );

unlink($filename);
rmdir($dirname);
?>
===DONE===