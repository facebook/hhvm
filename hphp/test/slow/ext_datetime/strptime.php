<?php

$format = "%d/%m/%Y %H:%M:%S";
$strf = strftime($format, strtotime("10/03/2004 15:54:19"));
var_dump($strf);
var_dump(strptime($strf, $format));
