<?php 
$path = str_repeat('a', PHP_MAXPATHLEN + 1);
var_dump(xmlwriter_open_uri('file:///' . $path));
?>