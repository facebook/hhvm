<?php
date_default_timezone_set('UTC');

$t = mktime(0,0,0, 6, 27, 2006);
var_dump(getdate(1,1));

var_dump(getdate($t));
var_dump(getdate());

echo "Done\n";
?>