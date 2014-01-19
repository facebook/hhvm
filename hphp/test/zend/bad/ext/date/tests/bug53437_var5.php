<?php
$s = 'O:12:"DateInterval":15:{s:1:"y";s:1:"2";s:1:"m";s:1:"0";s:1:"d";s:3:"bla";s:1:"h";s:1:"6";s:1:"i";s:1:"8";s:1:"s";s:1:"0";s:7:"weekday";i:10;s:16:"weekday_behavior";i:10;s:17:"first_last_day_of";i:0;s:6:"invert";i:0;s:4:"days";s:4:"aoeu";s:12:"special_type";i:0;s:14:"special_amount";s:21:"234523452345234532455";s:21:"have_weekday_relative";i:21474836489;s:21:"have_special_relative";s:3:"bla";}';

$di = unserialize($s);
var_dump($di);

?>
==DONE==