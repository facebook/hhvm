<?php
// build a predictable string
$string = '';
for($i=0; $i<30000; ++$i) $string .= $i . ' ';
var_dump(strlen($string));
// deflate string
$deflated = gzdeflate($string,9);
var_dump(strlen($deflated));
// truncate $deflated string
$truncated = substr($deflated, 0, 65535);
var_dump(strlen($truncated));
// inflate $truncated string (check if it will not eat all memory)
var_dump(gzinflate($truncated));
?>