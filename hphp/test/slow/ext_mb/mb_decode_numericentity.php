<?php

$convmap = array(0x0, 0x2FFFF, 0, 0xFFFF);
var_dump(mb_decode_numericentity("&#8217;&#7936;&#226;", $convmap, "UTF-8") ===
     "\xe2\x80\x99\xe1\xbc\x80\xc3\xa2");

$convmap = array(0x0, 0x2FFFF, 0, 0xFFFF);
var_dump(mb_encode_numericentity("\xe2\x80\x99\xe1\xbc\x80\xc3\xa2",
                                 $convmap, "UTF-8") ===
   "&#8217;&#7936;&#226;");
