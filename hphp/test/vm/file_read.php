<?php
$f = tmpfile();
fwrite($f, "foo.bar.baz");
fseek($f, 0);
var_dump(fread($f, 3));
fseek($f, 1, SEEK_CUR);
var_dump(fread($f, 3));
fseek($f, 1, SEEK_CUR);
var_dump(fread($f, 3));
fseek($f, 0, SEEK_SET);
var_dump(fread($f,3));
fwrite($f,"guzzle");
fseek($f, 0);
var_dump(fread($f,1000));
fclose($f);
