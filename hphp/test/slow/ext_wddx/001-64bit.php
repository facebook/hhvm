<?php
$path = dirname(__FILE__);
var_dump(wddx_deserialize(file_get_contents("{$path}/wddx.xml")));
