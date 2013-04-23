<?php
// non-strict mode
var_dump(mb_detect_encoding("A\x81", "SJIS", false));
// strict mode
var_dump(mb_detect_encoding("A\x81", "SJIS", true));
// non-strict mode
var_dump(mb_detect_encoding("\xc0\x00", "UTF-8", false));
// strict mode
var_dump(mb_detect_encoding("\xc0\x00", "UTF-8", true));
?>