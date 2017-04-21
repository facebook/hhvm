<?php
var_dump(parse_url('http://www.facebook.com'));
var_dump(parse_url('https://lol:12345@www.facebook.com:14159/hhvmabcd'));
var_dump(parse_url('irc://chat.freenode.net/#hhvm'));
var_dump(parse_url('content/:/\*'));
var_dump(parse_url("//example.org:8088/sites/default/files/drums.mp3"));
