<?php
var_dump(unpack("a4", "str\0\0"));
var_dump(unpack("a5", "str\0\0"));
var_dump(unpack("a6", "str\0\0"));
var_dump(unpack("a*", "str\0\0"));
?>