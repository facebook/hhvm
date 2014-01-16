<?php
$conf = opcache_get_configuration();
$conf = $conf['blacklist'];
$conf[3] = preg_replace("!^\\Q".dirname(__FILE__)."\\E!", "__DIR__", $conf[3]); 
$conf[4] = preg_replace("!^\\Q".dirname(__FILE__)."\\E!", "__DIR__", $conf[4]); 
print_r($conf);
include("blacklist.inc");
$status = opcache_get_status();
print_r(count($status['scripts']));
?>