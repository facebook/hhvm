<?php 
include 'bug64482.inc';
echo "\n";
include 'php://filter/read=string.toupper/resource=bug64482.inc';
echo "\n";
?>