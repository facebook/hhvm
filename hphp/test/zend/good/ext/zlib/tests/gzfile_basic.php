<?php
$plaintxt = b<<<EOT
hello world
is a very common test
for all languages
EOT;
$dirname = 'gzfile_temp';
$filename = $dirname.'/gzfile_basic.txt.gz';
mkdir($dirname);
$h = gzopen($filename, 'w');
gzwrite($h, $plaintxt);
gzclose($h);


var_dump(gzfile( $filename ) );

unlink($filename);
rmdir($dirname);
?>
===DONE===