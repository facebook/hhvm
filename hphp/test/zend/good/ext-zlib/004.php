<?php

var_dump(gzfile());
var_dump(gzfile("nonexistent_file_gzfile",1));
var_dump(gzfile(1,1,1));

var_dump(gzfile(dirname(__FILE__)."/004.txt.gz"));
var_dump(gzfile(dirname(__FILE__)."/004.txt.gz", 1));

echo "Done\n";
?>