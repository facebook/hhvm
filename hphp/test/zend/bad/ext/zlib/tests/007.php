<?php

var_dump(gzencode());
var_dump(gzencode(1,1,1,1));
var_dump(gzencode("", -10));
var_dump(gzencode("", 100));
var_dump(gzencode("", 1, 100));

var_dump(gzencode("", -1, ZLIB_ENCODING_GZIP));
var_dump(gzencode("", 9, ZLIB_ENCODING_DEFLATE));

$string = "Light of my sun
Light in this temple
Light in my truth
Lies in the darkness";

var_dump(gzencode($string, 9, 3));

var_dump(gzencode($string, -1, ZLIB_ENCODING_GZIP));
var_dump(gzencode($string, 9, ZLIB_ENCODING_DEFLATE));

echo "Done\n";
?>