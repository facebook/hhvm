<?php

var_dump(filter_var('0b15:23::3:67.98.234.17', FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var('::67:78b:34.43.43.2', FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));

?>