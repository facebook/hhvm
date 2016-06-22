<?php
exec('/bin/echo -n -e "line1\x0Cmoreline1\r\nline2\x0C"', $result, $ret);
$content = implode($result,"\n");
var_dump(bin2hex($content));
?>
