<?php
echo "no wrapper\n";
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f,'r');	
var_dump(stream_get_meta_data($h));
gzclose($h);
echo "\nwith wrapper\n";
$f = "compress.zlib://".dirname(__FILE__)."/004.txt.gz";
$h = fopen($f,'r');	
var_dump(stream_get_meta_data($h));
gzclose($h);


?>
===DONE===