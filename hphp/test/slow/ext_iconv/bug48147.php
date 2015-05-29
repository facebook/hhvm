<?php
$text = "aa\xC3\xC3\xC3\xB8aa";
var_dump(iconv("UTF-8", "UTF-8", $text));
var_dump(urlencode(iconv("UTF-8", "UTF-8//IGNORE", $text)));
// only invalid
var_dump(urlencode(iconv("UTF-8", "UTF-8//IGNORE", "\xC3")));
// start invalid
var_dump(urlencode(iconv("UTF-8", "UTF-8//IGNORE", "\xC3\xC3\xC3\xB8aa")));
// finish invalid
var_dump(urlencode(iconv("UTF-8", "UTF-8//IGNORE", "aa\xC3\xC3\xC3")));
