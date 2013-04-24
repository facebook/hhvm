<?php

$dir = opendir(dirname(__FILE__));
var_dump(stream_get_meta_data($dir));
closedir($dir);

$dirObject = dir(dirname(__FILE__));
var_dump(stream_get_meta_data($dirObject->handle));

?>