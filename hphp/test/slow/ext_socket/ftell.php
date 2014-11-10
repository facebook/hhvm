<?php
$s = fsockopen("udp://127.0.0.1", 12345);
var_dump(fwrite($s, "foomeme"));
var_dump(ftell($s));
var_dump(fseek($s, 7, SEEK_CUR));
var_dump(fseek($s, 3));
