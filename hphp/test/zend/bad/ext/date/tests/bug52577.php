<?php
date_default_timezone_set('Europe/Kiev');
$date = '7.8.2010';
echo "String: ".$date."\n";
$date_format = DATE_RFC2822;
$unixtime = strtotime($date);
echo "Unixtime: ".$unixtime."\n";
echo "Date(PHP): ".date($date_format,$unixtime)."\n";
$date = new DateTime('@'.$unixtime);
echo "DateTime(PHP Class): ".$date->format($date_format);
?>